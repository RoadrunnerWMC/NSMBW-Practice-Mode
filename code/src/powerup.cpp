#include "powerup.h"


// ref. P1:80a28430
#define DEFAULT_STAR_TIMER 660


// This little array defines a more logical order for powerup cycling (in my opinion):
// 1. POWERUP_SMALL
// 2. POWERUP_SUPER
// 3. POWERUP_FIRE
// 4. POWERUP_ICE
// 5. POWERUP_PROPELLER
// 6. POWERUP_PENGUIN
// 7. POWERUP_MINI
// It's implemented as a lookup table: at the index of your current
// powerup enum value is the next powerup enum value.
u8 next_powerup[] = {
    /* (enum value 0) 1. POWERUP_SMALL     -> 2. */ POWERUP_SUPER,
    /* (enum value 1) 2. POWERUP_SUPER     -> 3. */ POWERUP_FIRE,
    /* (enum value 2) 3. POWERUP_FIRE      -> 4. */ POWERUP_ICE,
    /* (enum value 3) 7. POWERUP_MINI      -> 1. */ POWERUP_SMALL,
    /* (enum value 4) 5. POWERUP_PROPELLER -> 6. */ POWERUP_PENGUIN,
    /* (enum value 5) 6. POWERUP_PENGUIN   -> 7. */ POWERUP_MINI,
    /* (enum value 6) 4. POWERUP_ICE       -> 5. */ POWERUP_PROPELLER,
};

void cycle_powerup(dAcPy_c *player) {
    player->setPowerupAlt((Powerup)(next_powerup[player->powerup]));
}

void toggle_star(dAcPy_c *player) {
    if (player->vtable->isStar(player)) {
        player->vtable->endStar(player);
    } else {
        player->vtable->startStar(player, 0, DEFAULT_STAR_TIMER);
    }
}
