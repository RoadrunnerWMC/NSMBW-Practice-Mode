#include "game.h"
#include "zone_control.h"


#define FAKE_ENTRANCE_ID 254

NextGoto FakeEntrance = {0};
bool ForceFacingLeft = false;

struct PlayerState {
    u32 lives;
    u32 coins;
    int powerup;
    int starTimer;
};

struct ZoneState {
    u32 numLivePlayers;
    u32 activePlayersBitfield;
    u32 score;
    u32 timerPreciseTime;
    u32 starCoinCollectionStates[3];
    PlayerState playerStates[4];
};

enum RestorationType {
    RESTORATION_TYPE_CURRENT_STAGE,
    RESTORATION_TYPE_CURRENT_ZONE,
};

ZoneState initial_state_of_current_stage;
ZoneState initial_state_of_current_zone;
bool is_restoring_stage = false;
bool is_restoring_players = false;
RestorationType current_restoration_type;


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


void save_zone_state(ZoneState *state) {
    state->numLivePlayers = daPyMng_c::mNum;
    state->activePlayersBitfield = daPyMng_c::mActPlayerInfo;
    state->playerStates[0].lives = daPyMng_c::mRest[0];
    state->playerStates[1].lives = daPyMng_c::mRest[1];
    state->playerStates[2].lives = daPyMng_c::mRest[2];
    state->playerStates[3].lives = daPyMng_c::mRest[3];
    state->playerStates[0].coins = daPyMng_c::mCoin[0];
    state->playerStates[1].coins = daPyMng_c::mCoin[1];
    state->playerStates[2].coins = daPyMng_c::mCoin[2];
    state->playerStates[3].coins = daPyMng_c::mCoin[3];
    state->score = daPyMng_c::mScore;
    state->timerPreciseTime = dStageTimer_c::m_instance->preciseTime;
    state->starCoinCollectionStates[0] = dScStage_c::mCollectionCoin[0];
    state->starCoinCollectionStates[1] = dScStage_c::mCollectionCoin[1];
    state->starCoinCollectionStates[2] = dScStage_c::mCollectionCoin[2];
}


void restore_zone_state(ZoneState *state) {
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

    // Reset all star coins to their original collection states
    dScStage_c::mCollectionCoin[0] = state->starCoinCollectionStates[0];
    dScStage_c::mCollectionCoin[1] = state->starCoinCollectionStates[1];
    dScStage_c::mCollectionCoin[2] = state->starCoinCollectionStates[2];

    // If a checkpoint is collected, clear it
    // (TODO: should really reset it to whatever it was at level load,
    // but it seems calling cyuukan->courseIN() doesn't do that)
    dInfo_c::m_instance->cyuukan.clear();

    // Reset daPyMng_c fields
    daPyMng_c::mNum = state->numLivePlayers;
    daPyMng_c::mActPlayerInfo = state->activePlayersBitfield;
    daPyMng_c::mRest[0] = state->playerStates[0].lives;
    daPyMng_c::mRest[1] = state->playerStates[1].lives;
    daPyMng_c::mRest[2] = state->playerStates[2].lives;
    daPyMng_c::mRest[3] = state->playerStates[3].lives;
    daPyMng_c::mCoin[0] = state->playerStates[0].coins;
    daPyMng_c::mCoin[1] = state->playerStates[1].coins;
    daPyMng_c::mCoin[2] = state->playerStates[2].coins;
    daPyMng_c::mCoin[3] = state->playerStates[3].coins;
    daPyMng_c::mScore = state->score;

    // Reset the timer time
    dStageTimer_c::m_instance->preciseTime = state->timerPreciseTime;

    // Reset dActorCreateMng_c (it gets disabled when the goal pole is
    // touched, so we'd like to re-enable it)
    dActorCreateMng_c::m_instance->ActorCreateInfoClear();
}


void save_zone_state_players(ZoneState *state) {
    for (int i = 0; i < 4; i++) {
        dAcPy_c *player = daPyMng_c::getPlayer(i);
        if (player == NULL)
            continue;

        state->playerStates[i].powerup = player->powerup;
        state->playerStates[i].starTimer = player->starTimer;
    }
}


void restore_zone_state_players(ZoneState *state) {
    for (int i = 0; i < 4; i++) {
        dAcPy_c *player = daPyMng_c::getPlayer(i);
        if (player == NULL)
            continue;

        player->setPowerupAlt((Powerup)(state->playerStates[i].powerup));

        u32 starTimer = state->playerStates[i].starTimer;
        if (starTimer == 0) {
            player->vtable->endStar(player);
        } else {
            player->vtable->startStar(player, 1, starTimer);
        }
    }
}


// Hook at the end of dScStage_c::create()
kmBranchDefCpp(0x80924e58, NULL, u32, ) {
    if (!is_restoring_stage) {
        if (dScStage_c::getCourseIn()) {
            save_zone_state(&initial_state_of_current_stage);
        }

        save_zone_state(&initial_state_of_current_zone);
    } else {
        if (current_restoration_type == RESTORATION_TYPE_CURRENT_ZONE) {
            restore_zone_state(&initial_state_of_current_zone);
        } else {
            restore_zone_state(&initial_state_of_current_stage);
        }

        is_restoring_stage = false;
    }
    return 1;
}

// Hook at the end of daPyMng_c::createCourseInit()
// (this hook needs to exist because players are created after
// dScStage_c::create() finishes executing)
kmBranchDefCpp(0x8005f4cc, NULL, void, ) {
    if (!is_restoring_players) {
        if (dScStage_c::getCourseIn()) {
            save_zone_state_players(&initial_state_of_current_stage);
        }

        save_zone_state_players(&initial_state_of_current_zone);
    } else {
        if (current_restoration_type == RESTORATION_TYPE_CURRENT_ZONE) {
            restore_zone_state_players(&initial_state_of_current_zone);
        } else {
            restore_zone_state_players(&initial_state_of_current_stage);
        }

        is_restoring_players = false;
    }
}


void trigger_stage_reload() {
    // Trigger a wipe
    dFader_c::setFader(dFader_c::FADER_TYPE_CIRCLE_5);

    // Force a background music fade-out
    // (note: "0" = the currently playing music, "1" = no music, and
    // higher values = specific music IDs)
    dAudio::hashname_a2bd17ff_6bcc38cc(1);

    // The game normally uses this function to reset some game state
    // upon death, including the spawn position if loading from
    // checkpoint
    dScStage_c::m_instance->restoreOldPlayerInfo();

    // Trigger the actual stage reload
    u8 world = dScStage_c::m_instance->curWorld;
    u8 level = dScStage_c::m_instance->curLevel;
    dInfo_c::m_instance->startGame((dInfo_c::StartGameInfo_s) {
        /* replay_duration */ 0,
        /* hint_movie_type */ 0,
        /* entrance */ 0xff,  // (default entrance)
        /* area */ 0,
        /* is_replay */ false,
        /* screen_type */ dInfo_c::SCREEN_TYPE_NORMAL,
        /* world_1 */ world,
        /* level_1 */ level,
        /* world_2 */ world,
        /* level_2 */ level
    });
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

    is_restoring_stage = true;
    is_restoring_players = true;
    current_restoration_type = RESTORATION_TYPE_CURRENT_ZONE;
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
