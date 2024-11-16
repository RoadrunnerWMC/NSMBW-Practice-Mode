#include "game.h"
#include "zone_reload.h"


#define FAKE_ENTRANCE_ID 254

NextGoto FakeEntrance = {0};
bool ForceFacingLeft = false;


struct ZoneState {
    u32 timerPreciseTime;
};

ZoneState initial_zone_state;
bool is_restoring = false;


u8 curEntranceNonFake = 0;
kmBranchDefAsm(0x809261bc, 0x809261c0) {
    nofralloc

    cmpwi r4, FAKE_ENTRANCE_ID
    beq fake

    lis r6, curEntranceNonFake@ha
    addi r6, r6, curEntranceNonFake@l
    stb r4, 0(r6);

fake:
    stb r4, 0x1211(r3)  // original instruction
    blr
}


void save_initial_zone_state() {
    initial_zone_state.timerPreciseTime = dStageTimer_c::m_instance->preciseTime;
}


void restore_initial_zone_state() {
    // Reset stored sprite data in dActorCreateMng_c
    // (resets spawn status for various types of enemies, mainly)
    // TODO: maybe just reset this for only the current zone, instead of
    // the whole level?
    // Since storedShorts and storedBytes are adjacent in memory, for
    // efficiency, I just clear both of them with one call
    memset(&dActorCreateMng_c::m_instance->storedShorts, 0, 2000 + 1000);

    // Reset the bitfield arrays in dBgParameter_c
    // (resets spawn status for various types of coins and blocks)
    for (int i = 0; i < 12; i++) {
        memset(dBgParameter_c::ms_Instance_p->tileBuffers[i], 0, 0x10000);
    }
    // TODO: this doesn't affect tile-based coins/blocks until the area
    // is reloaded
    // TODO: maybe just reset this for only the current zone, instead of
    // the whole level?

    // Reset dFlagCtrl_c
    // (resets spawn status for star coins, red rings, roulette blocks,
    // etc.)
    dFlagCtrl_c::m_instance->clearAllFlagData();
    // TODO: maybe just reset this for only the current zone, instead of
    // the whole level?

    // Reset all star coins to uncollected
    dScStage_c::mCollectionCoin[0] = 4;
    dScStage_c::mCollectionCoin[1] = 4;
    dScStage_c::mCollectionCoin[2] = 4;

    // If a checkpoint is collected, clear it
    // (TODO: should really reset it to whatever it was at level load,
    // but it seems calling cyuukan->courseIN() doesn't do that)
    dInfo_c::m_instance->cyuukan.clear();

    // Reset the timer time
    dStageTimer_c::m_instance->preciseTime = initial_zone_state.timerPreciseTime;
}


// Hook at the end of dScStage_c::create()
kmBranchDefCpp(0x80924e58, NULL, u32, ) {
    if (!is_restoring) {
        save_initial_zone_state();
    } else {
        restore_initial_zone_state();
        is_restoring = false;
    }
    return 1;
}


void trigger_zone_reload(daPlBase_c *player, bool keep_current_pos) {
    if (keep_current_pos) {
        FakeEntrance.x = player->pos.x - 8.0f;
        FakeEntrance.y = -(player->pos.y + 16.0f);
        FakeEntrance.zone_id = dScStage_c::m_instance->curZone;
        FakeEntrance.dest_id = FAKE_ENTRANCE_ID;
        ForceFacingLeft = (player->direction == 1);
    } else {
        FakeEntrance.dest_id = curEntranceNonFake;
    }

    player->useNextGotoBlock(FAKE_ENTRANCE_ID, 0, 0);

    is_restoring = true;
}


extern "C" NextGoto *getNextGotoP__9dCdFile_cFUc_PLUS_FOUR(u8 id);

kmBranchDefAsm(0x8008e3d0, NULL) {
    nofralloc

    cmpwi r4, FAKE_ENTRANCE_ID
    beq fake
    lwz r0, 0x90(r3)  // original instruction
    b getNextGotoP__9dCdFile_cFUc_PLUS_FOUR

fake:
    lis r3, FakeEntrance@h
    ori r3, r3, FakeEntrance@l
    blr
}


// TODO: this approach doesn't really work for multiplayer
kmBranchDefAsm(0x8005ef08, 0x8005ef0c) {
    nofralloc

    lis r4, ForceFacingLeft@ha
    lbz r0, ForceFacingLeft@l(r4)
    cmpwi r0, 0
    beq finish
    li r0, 0
    stb r0, ForceFacingLeft@l(r4)
    li r6, 1

finish:
    clrlwi r4, r3, 28  // original instruction
    blr
}
