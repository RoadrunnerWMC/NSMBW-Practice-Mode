#include "game.h"
#include "savestate_lite.h"
#include "powerup.h"
#include "zone_reload.h"


bool SavedStateValid = false;  // TODO: reset to false when loading into the level
SavestateLite SavedState = {0};


#define SE_SYS_INVALID 123
#define SE_SYS_ROUTE_OK 187


// Note: this enum is only valid for input bitfields before dGameKeyCore_c::setConfigKey()
enum Buttons {
    // bit        sideways     nunchuk
    // 0000_0001  d-pad down   d-pad left
    // 0000_0002  d-pad up     d-pad right
    // 0000_0004  d-pad right  d-pad down
    // 0000_0008  d-pad left   d-pad up
    // 0000_0010  plus         plus
    // 0000_0100  2            2
    // 0000_0200  1            1
    // 0000_0400  B            B
    // 0000_0800  A            A
    // 0000_1000  minus        minus
    // 0000_2000               Z
    // 0000_4000               C
    // 0000_8000  home         home
    // 0001_0000               nunchuk up
    // 0002_0000               nunchuk down
    // 0004_0000               nunchuk left
    // 0008_0000               nunchuk right
    BUTTON_DPAD_LEFT = 0x1,
    BUTTON_DPAD_RIGHT = 0x2,
    BUTTON_DPAD_DOWN = 0x4,
    BUTTON_DPAD_UP = 0x8,
    BUTTON_PLUS = 0x10,
    BUTTON_2 = 0x100,
    BUTTON_1 = 0x200,
    BUTTON_B = 0x400,
    BUTTON_A = 0x800,
    BUTTON_MINUS = 0x1000,
    BUTTON_Z = 0x2000,
    BUTTON_C = 0x4000,
    BUTTON_HOME = 0x8000,
    BUTTON_NUNCHUK_UP = 0x10000,
    BUTTON_NUNCHUK_DOWN = 0x20000,
    BUTTON_NUNCHUK_LEFT = 0x40000,
    BUTTON_NUNCHUK_RIGHT = 0x80000,
};

enum VirtualButtons {
    VBUTTON_SAVESTATE_SAVE = 0x1,
    VBUTTON_SAVESTATE_RESTORE = 0x2,
    VBUTTON_CYCLE_POWERUP = 0x4,
    VBUTTON_RELOAD_ZONE_START = 0x8,
    VBUTTON_RELOAD_ZONE_HERE = 0x10,
};



u32 TruePrevButtonsHeld[4] = {0};
u32 VirtualButtonsHeld[4] = {0};
u32 VirtualButtonsPressed[4] = {0};
bool MinusComboWasInput[4] = {0};
u32 SuppressedInputs[4] = {0};

u32 dGameKeyCore_c_intercept_input(dGameKeyCore_c *this_, u32 bitfield) {

    u32 id = (u32)this_->id;
    u32 *true_prev_buttons_held = &TruePrevButtonsHeld[id];
    u32 *virtuals_held = &VirtualButtonsHeld[id];
    u32 *virtuals_pressed = &VirtualButtonsPressed[id];
    bool *minus_combo_was_input = &MinusComboWasInput[id];
    u32 *suppression = &SuppressedInputs[id];

    u32 virtuals_prev_held = *virtuals_held;

    if (bitfield & BUTTON_MINUS) {
        *suppression |= BUTTON_MINUS;

        if (this_->controllerType == SIDEWAYS) {
            if (bitfield & BUTTON_1) {
                *virtuals_held |= VBUTTON_RELOAD_ZONE_START;
                *suppression |= BUTTON_1;
                *minus_combo_was_input = true;
            } else {
                *virtuals_held &= ~VBUTTON_RELOAD_ZONE_START;
            }

            if (bitfield & BUTTON_2) {
                *virtuals_held |= VBUTTON_RELOAD_ZONE_HERE;
                *suppression |= BUTTON_2;
                *minus_combo_was_input = true;
            } else {
                *virtuals_held &= ~VBUTTON_RELOAD_ZONE_HERE;
            }

            if (bitfield & BUTTON_A) {
                *virtuals_held |= VBUTTON_CYCLE_POWERUP;
                *suppression |= BUTTON_A;
                *minus_combo_was_input = true;
            } else {
                *virtuals_held &= ~VBUTTON_CYCLE_POWERUP;
            }

        } else {
            if (bitfield & BUTTON_B) {
                *virtuals_held |= VBUTTON_RELOAD_ZONE_START;
                *suppression |= BUTTON_B;
                *minus_combo_was_input = true;
            } else {
                *virtuals_held &= ~VBUTTON_RELOAD_ZONE_START;
            }

            if (bitfield & BUTTON_A) {
                *virtuals_held |= VBUTTON_RELOAD_ZONE_HERE;
                *suppression |= BUTTON_A;
                *minus_combo_was_input = true;
            } else {
                *virtuals_held &= ~VBUTTON_RELOAD_ZONE_HERE;
            }

            if (bitfield & BUTTON_Z) {
                *virtuals_held |= VBUTTON_CYCLE_POWERUP;
                *suppression |= BUTTON_Z;
                *minus_combo_was_input = true;
            } else {
                *virtuals_held &= ~VBUTTON_CYCLE_POWERUP;
            }
        }
    } else {
        if (*true_prev_buttons_held & BUTTON_MINUS) {
            *virtuals_held = 0;
            if (*minus_combo_was_input) {
                *minus_combo_was_input = false;
            } else {
                *virtuals_held |= VBUTTON_SAVESTATE_SAVE;
            }
        } else {
            *virtuals_held &= ~VBUTTON_SAVESTATE_SAVE;
        }
    }

    if (this_->controllerType == SIDEWAYS) {
        if (bitfield & BUTTON_B) {
            *virtuals_held |= VBUTTON_SAVESTATE_RESTORE;
            *suppression |= BUTTON_B;
        } else {
            *virtuals_held &= ~VBUTTON_SAVESTATE_RESTORE;
        }
    } else {
        if (bitfield & BUTTON_C) {
            *virtuals_held |= VBUTTON_SAVESTATE_RESTORE;
            *suppression |= BUTTON_C;
        } else {
            *virtuals_held &= ~VBUTTON_SAVESTATE_RESTORE;
        }
    }

    // Recalculate "pressed" virtual buttons
    *virtuals_pressed = *virtuals_held & (*virtuals_held ^ virtuals_prev_held);

    // Suppress certain bits in the bitfield, and also remove
    // suppressed bits that aren't being held anymore
    *true_prev_buttons_held = bitfield;
    *suppression &= bitfield;
    bitfield &= ~*suppression;

    return bitfield;
}

kmBranchDefAsm(0x800b5e30, 0x800b5e34) {
    nofralloc         // no prologue/epilogue
    clrlwi r0, r0, 1  // original instruction

    // SAVE
    subi r1, r1, 0x30
    stw r3, 0x00(r1)
    stw r4, 0x04(r1)
    stw r5, 0x08(r1)
    stw r6, 0x0c(r1)
    stw r7, 0x10(r1)
    stw r8, 0x14(r1)
    stw r9, 0x18(r1)
    stw r10, 0x1c(r1)
    stw r11, 0x20(r1)
    stw r12, 0x24(r1)

    mr r4, r0
    bl dGameKeyCore_c_intercept_input
    mr r0, r3

    // RESTORE
    lwz r3, 0x00(r1)
    lwz r4, 0x04(r1)
    lwz r5, 0x08(r1)
    lwz r6, 0x0c(r1)
    lwz r7, 0x10(r1)
    lwz r8, 0x14(r1)
    lwz r9, 0x18(r1)
    lwz r10, 0x1c(r1)
    lwz r11, 0x20(r1)
    lwz r12, 0x24(r1)
    addi r1, r1, 0x30

    blr  // replaced with branch to exit point by Kamek
}


extern "C" int daPlBase_c_execute(daPlBase_c *this_);

int dAcPy_c_execute_wrapper(dAcPy_c *this_) {
    int res = daPlBase_c_execute(this_);

    if (this_->input.remoconID != -1) {
        u32 array_id = (u32)(dGameKey_c::m_instance->remocons[this_->input.remoconID]->id);
        u32 virtual_buttons_pressed = VirtualButtonsPressed[array_id];

        if (virtual_buttons_pressed & VBUTTON_SAVESTATE_SAVE) {
            save_state(this_, &SavedState);
            SavedStateValid = true;

            this_->playSound(SE_SYS_ROUTE_OK, 1);
        }

        if (virtual_buttons_pressed & VBUTTON_SAVESTATE_RESTORE) {
            if (SavedStateValid) {
                restore_state(this_, &SavedState);
            } else {
                this_->playSound(SE_SYS_INVALID, 1);
            }
        }

        if (virtual_buttons_pressed & VBUTTON_CYCLE_POWERUP) {
            cycle_powerup(this_);
            // or: toggle_star(this_);
        }

        if (virtual_buttons_pressed & VBUTTON_RELOAD_ZONE_START) {
            trigger_zone_reload(this_, false);
        }

        if (virtual_buttons_pressed & VBUTTON_RELOAD_ZONE_HERE) {
            trigger_zone_reload(this_, true);
        }
    }

    return res;
}

kmWritePointer(0x80325788, dAcPy_c_execute_wrapper);
