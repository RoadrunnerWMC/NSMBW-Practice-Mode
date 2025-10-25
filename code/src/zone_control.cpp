#include "game.h"
#include "zone_control.h"


#define FAKE_ENTRANCE_ID 254

NextGoto FakeEntrance = {0};
bool ForceFacingLeft = false;

struct STATE_dScStage_c {
    u32 mCollectionCoin[3];  // star coin collection states
};

struct STATE_daPyMng_c {
    // (Arrays of length 4 are per-player)
    u32 m_yoshiFruit[4];  // number of Yoshi fruits collected (0-4)
    u32 mPlayerType[4];   // which character this player is playing as
    u32 mPlayerMode[4];   // powerup
    u32 mCreateItem[4];   // flags indicating whether the player is in a
                          // bubble and whether they're riding Yoshi
    u32 mRest[4];         // number of lives
    u32 mCoin[4];         // number of coins (0-99)
    u32 mNum;             // number of live players
    u32 mActPlayerInfo;   // bitfield of active players
    u8 m_yoshiColor[4];
    u16 m_star_time[4];   // amount of invincibility time left
    u16 m_star_count[4];  // number of enemies player has killed since
                          // becoming invincible (for determining points
                          // and 1UPs)
    u32 mScore;
};

struct STATE_dStageTimer_c {
    u32 preciseTime;
};

struct ZoneState {
    STATE_dScStage_c dScStage_c;
    STATE_daPyMng_c daPyMng_c;
    STATE_dStageTimer_c dStageTimer_c;
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


void save_dScStage_c(STATE_dScStage_c *state) {
    state->mCollectionCoin[0] = dScStage_c::mCollectionCoin[0];
    state->mCollectionCoin[1] = dScStage_c::mCollectionCoin[1];
    state->mCollectionCoin[2] = dScStage_c::mCollectionCoin[2];
}


void save_daPyMng_c(STATE_daPyMng_c *state) {
    // Have each player update its relevant fields first
    for (int i = 0; i < 4; i++) {
        dAcPy_c *player = daPyMng_c::getPlayer(i);
        if (player != NULL) {
            player->setSceneChangeInfo();
        }
    }

    state->mNum = daPyMng_c::mNum;
    state->mActPlayerInfo = daPyMng_c::mActPlayerInfo;
    state->mScore = daPyMng_c::mScore;

    for (int i = 0; i < 4; i++) {
        state->mRest[i] = daPyMng_c::mRest[i];
        state->mCoin[i] = daPyMng_c::mCoin[i];
        state->m_yoshiFruit[i] = daPyMng_c::m_yoshiFruit[i];
        state->mPlayerMode[i] = daPyMng_c::mPlayerMode[i];
        state->mCreateItem[i] = daPyMng_c::mCreateItem[i];
        state->m_star_time[i] = daPyMng_c::m_star_time[i];
        state->m_star_count[i] = daPyMng_c::m_star_count[i];
        state->mPlayerType[i] = daPyMng_c::mPlayerType[i];
        state->m_yoshiColor[i] = daPyMng_c::m_yoshiColor[i];
    }
}


void save_dStageTimer_c(STATE_dStageTimer_c *state) {
    state->preciseTime = dStageTimer_c::m_instance->preciseTime;
}


void save_zone_state_early(ZoneState *state) {
    save_dScStage_c(&state->dScStage_c);
    save_daPyMng_c(&state->daPyMng_c);
}


void save_zone_state_late(ZoneState *state) {
    save_dStageTimer_c(&state->dStageTimer_c);
}


// TOOD/FIXME: if you save without Yoshi, then reset while on Yoshi, the
// bongos keep playing


void restore_dScStage_c(STATE_dScStage_c *state) {
    dScStage_c::mCollectionCoin[0] = state->mCollectionCoin[0];
    dScStage_c::mCollectionCoin[1] = state->mCollectionCoin[1];
    dScStage_c::mCollectionCoin[2] = state->mCollectionCoin[2];
}


void restore_daPyMng_c(STATE_daPyMng_c *state) {
    daPyMng_c::mNum = state->mNum;
    daPyMng_c::mActPlayerInfo = state->mActPlayerInfo;
    daPyMng_c::mScore = state->mScore;

    for (int i = 0; i < 4; i++) {
        daPyMng_c::mRest[i] = state->mRest[i];
        daPyMng_c::mCoin[i] = state->mCoin[i];
        daPyMng_c::m_yoshiFruit[i] = state->m_yoshiFruit[i];
        daPyMng_c::mPlayerMode[i] = state->mPlayerMode[i];
        daPyMng_c::mCreateItem[i] = state->mCreateItem[i];
        daPyMng_c::m_star_time[i] = state->m_star_time[i];
        daPyMng_c::m_star_count[i] = state->m_star_count[i];
        daPyMng_c::mPlayerType[i] = state->mPlayerType[i];
        daPyMng_c::m_yoshiColor[i] = state->m_yoshiColor[i];
    }
}


void restore_dCyuukan_c() {
    // If a checkpoint is collected, clear it
    // (TODO: should really reset it to whatever it was at level load,
    // but it seems calling cyuukan->courseIN() doesn't do that)
    dInfo_c::m_instance->cyuukan.clear();
}


void restore_dStageTimer_c(STATE_dStageTimer_c *state) {
    dStageTimer_c::m_instance->preciseTime = state->preciseTime;
}


void restore_dActorCreateMng_c() {
    // Reset stored sprite data in dActorCreateMng_c
    // (resets spawn status for various types of enemies, mainly)
    // TODO: maybe just reset this for only the current zone, instead of
    // the whole level?
    // Since storedShorts and storedBytes are adjacent in memory, for
    // efficiency, I just clear both of them with one call
    memset(&dActorCreateMng_c::m_instance->storedShorts, 0, 2000 + 1000);

    // Reset dActorCreateMng_c itself (it gets disabled when the goal
    // pole is touched, so we'd like to re-enable it)
    dActorCreateMng_c::m_instance->ActorCreateInfoClear();
}


void restore_dBgParameter_c() {
    // Reset the bitfield arrays in dBgParameter_c
    // (resets spawn status for various types of coins and blocks)
    for (int i = 0; i < 12; i++) {
        memset(dBgParameter_c::ms_Instance_p->tileBuffers[i], 0, 0x10000);
    }
    // TODO: this doesn't affect tile-based coins/blocks until the area
    // is reloaded
    // TODO: maybe just reset this for only the current zone, instead of
    // the whole level?
}


void restore_dFlagCtrl_c() {
    // Reset dFlagCtrl_c
    // (resets spawn status for star coins, red rings, roulette blocks,
    // etc.)
    dFlagCtrl_c::m_instance->clearAllFlagData();
    // TODO: maybe just reset this for only the current zone, instead of
    // the whole level?
}


void restore_zone_state_early(ZoneState *state) {
    restore_dScStage_c(&state->dScStage_c);
    restore_daPyMng_c(&state->daPyMng_c);
    restore_dCyuukan_c();
}


void restore_zone_state_late(ZoneState *state) {
    restore_dStageTimer_c(&state->dStageTimer_c);
    restore_dActorCreateMng_c();
    restore_dBgParameter_c();
    restore_dFlagCtrl_c();
}


extern "C" int dScStage_c_create(dScStage_c *this_);

int dScStage_c_create_wrapper(dScStage_c *this_) {
    // TODO: is there a way to make this logic less repetitive?

    // Save or restore all info that must be handled *before*
    // dScStage_c::create() runs
    if (!is_restoring_stage) {
        if (dScStage_c::getCourseIn()) {
            save_zone_state_early(&initial_state_of_current_stage);
        }

        save_zone_state_early(&initial_state_of_current_zone);
    } else {
        if (current_restoration_type == RESTORATION_TYPE_CURRENT_ZONE) {
            restore_zone_state_early(&initial_state_of_current_zone);
        } else {
            restore_zone_state_early(&initial_state_of_current_stage);
        }
    }

    // Run dScStage_c::create()
    int res = dScStage_c_create(this_);

    // Save or restore all info that must be handled *after*
    // dScStage_c::create() runs
    if (!is_restoring_stage) {
        if (dScStage_c::getCourseIn()) {
            save_zone_state_late(&initial_state_of_current_stage);
        }

        save_zone_state_late(&initial_state_of_current_zone);
    } else {
        if (current_restoration_type == RESTORATION_TYPE_CURRENT_ZONE) {
            restore_zone_state_late(&initial_state_of_current_zone);
        } else {
            restore_zone_state_late(&initial_state_of_current_stage);
        }

        is_restoring_stage = false;
    }

    return res;
}

kmWritePointer(0x8098dc30, dScStage_c_create_wrapper);


void trigger_stage_reload() {
    if (dScStage_c::m_instance == NULL) {
        return;
    }

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


void trigger_zone_reload(dAcPy_c *player, bool keep_current_pos) {
    if (keep_current_pos) {
        FakeEntrance.x = player->pos.x - 8.0f;
        FakeEntrance.y = -(player->pos.y + 16.0f);
        FakeEntrance.zone_id = dScStage_c::m_instance->curZone;
        FakeEntrance.dest_id = FAKE_ENTRANCE_ID;
        ForceFacingLeft = (player->direction == 1);
    } else {
        FakeEntrance.dest_id = curEntranceNonFake;
    }

    dNext_c::m_instance->initGoto(
        dScStage_c::m_instance->curArea,
        FAKE_ENTRANCE_ID,
        dFader_c::FADER_TYPE_WAVY
    );
    dNext_c::m_instance->m_timer = 0;
    player->vtable->changeNextScene(player, 0);

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
