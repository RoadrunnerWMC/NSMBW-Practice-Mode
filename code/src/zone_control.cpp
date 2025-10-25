#include "game.h"
#include "zone_control.h"


#define FAKE_ENTRANCE_ID 254

NextGoto FakeEntrance = {0};
bool ForceFacingLeft = false;

struct STATE_dScStage_c {
    u32 mCollectionCoin[3];  // star coin collection states
};

struct STATE_daPyMng_c {
    u32 m_yoshiFruit[NUM_PLAYERS];  // number of Yoshi fruits collected (0-4)
    u32 mPlayerType[NUM_PLAYERS];   // which character this player is playing as
    u32 mPlayerMode[NUM_PLAYERS];   // powerup
    u32 mCreateItem[NUM_PLAYERS];   // flags indicating whether the player is in a
                                    // bubble and whether they're riding Yoshi
    u32 mRest[NUM_PLAYERS];         // number of lives
    u32 mCoin[NUM_PLAYERS];         // number of coins (0-99)
    u32 mNum;                       // number of live players
    u32 mActPlayerInfo;             // bitfield of active players
    u8 m_yoshiColor[NUM_PLAYERS];
    u16 m_star_time[NUM_PLAYERS];   // amount of invincibility time left
    u16 m_star_count[NUM_PLAYERS];  // number of enemies player has killed since
                                    // becoming invincible (for determining points
                                    // and 1UPs)
    u32 mScore;
};

struct STATE_dCyuukan_c {
    Vec playerSpawnPos;
    u32 curWorldLevelAreaAndEntrance;
    bool isKinopioInChukan;
    u32 starCoinStatus[3];
};

struct STATE_dStageTimer_c {
    u32 preciseTime;
};

struct STATE_dActorCreateMng_c {
    // This struct MUST exactly match the beginning of dActorCreateMng_c
    // itself, since we currently just do a single memcpy() to copy
    // between the two. Be careful if editing this struct!
    u32 counters[4];
    u16 storedShorts[1000];
    u8 storedBytes[1000];
    u16 unkBC8;
    bool isEndingDemo;
    bool stopped;
};

struct STATE_dFlagCtrl_c {
    // This struct MUST exactly match the beginning of dFlagCtrl_c
    // itself, since we currently just do a single memcpy() to copy
    // between the two. Be careful if editing this struct!
    u16 items[128];
};

struct ZoneState {
    // Saved/restored "early" (before dScStage_c::create())
    STATE_dScStage_c dScStage_c;
    STATE_daPyMng_c daPyMng_c;
    STATE_dCyuukan_c dCyuukan_c;

    // Saved/restored "late" (after dScStage_c::create())
    STATE_dStageTimer_c dStageTimer_c;
    STATE_dActorCreateMng_c dActorCreateMng_c;
    STATE_dFlagCtrl_c dFlagCtrl_c;
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
    for (int i = 0; i < NUM_PLAYERS; i++) {
        dAcPy_c *player = daPyMng_c::getPlayer(i);
        if (player != NULL) {
            player->setSceneChangeInfo();
        }
    }

    state->mNum = daPyMng_c::mNum;
    state->mActPlayerInfo = daPyMng_c::mActPlayerInfo;
    state->mScore = daPyMng_c::mScore;

    for (int i = 0; i < NUM_PLAYERS; i++) {
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


void save_dCyuukan_c(STATE_dCyuukan_c *state) {
    dCyuukan_c *cyuukan = &dInfo_c::m_instance->cyuukan;
    state->playerSpawnPos = cyuukan->playerSpawnPos;
    state->curWorldLevelAreaAndEntrance = cyuukan->curWorldLevelAreaAndEntrance;
    state->isKinopioInChukan = cyuukan->isKinopioInChukan;
    state->starCoinStatus[0] = cyuukan->starCoinStatus[0];
    state->starCoinStatus[1] = cyuukan->starCoinStatus[1];
    state->starCoinStatus[2] = cyuukan->starCoinStatus[2];
}


void save_dStageTimer_c(STATE_dStageTimer_c *state) {
    state->preciseTime = dStageTimer_c::m_instance->preciseTime;
}


void save_dActorCreateMng_c(STATE_dActorCreateMng_c *state) {
    // We save the first 0xbcc bytes of dActorCreateMng exactly as-is,
    // so let's just do a memcpy
    memcpy(state, dActorCreateMng_c::m_instance, sizeof(STATE_dActorCreateMng_c));
}


void save_dFlagCtrl_c(STATE_dFlagCtrl_c *state) {
    // We save all 0x100 bytes of dFlagCtrl_c exactly as-is, so let's
    // just do a memcpy
    memcpy(state, dFlagCtrl_c::m_instance, sizeof(STATE_dFlagCtrl_c));
}


void save_zone_state_early(ZoneState *state) {
    save_dScStage_c(&state->dScStage_c);
    save_daPyMng_c(&state->daPyMng_c);
    save_dCyuukan_c(&state->dCyuukan_c);
}


void save_zone_state_late(ZoneState *state) {
    save_dStageTimer_c(&state->dStageTimer_c);
    save_dActorCreateMng_c(&state->dActorCreateMng_c);
    save_dFlagCtrl_c(&state->dFlagCtrl_c);
}


void restore_dScStage_c(STATE_dScStage_c *state) {
    dScStage_c::mCollectionCoin[0] = state->mCollectionCoin[0];
    dScStage_c::mCollectionCoin[1] = state->mCollectionCoin[1];
    dScStage_c::mCollectionCoin[2] = state->mCollectionCoin[2];
}


void restore_daPyMng_c(STATE_daPyMng_c *state) {
    daPyMng_c::mNum = state->mNum;
    daPyMng_c::mActPlayerInfo = state->mActPlayerInfo;
    daPyMng_c::mScore = state->mScore;

    bool enable_yoshi_drums = false;

    for (int i = 0; i < NUM_PLAYERS; i++) {
        daPyMng_c::mRest[i] = state->mRest[i];
        daPyMng_c::mCoin[i] = state->mCoin[i];
        daPyMng_c::m_yoshiFruit[i] = state->m_yoshiFruit[i];
        daPyMng_c::mPlayerMode[i] = state->mPlayerMode[i];
        daPyMng_c::mCreateItem[i] = state->mCreateItem[i];
        if (state->mCreateItem[i] == 2) {  // player is riding Yoshi
            enable_yoshi_drums = true;
        }
        daPyMng_c::m_star_time[i] = state->m_star_time[i];
        daPyMng_c::m_star_count[i] = state->m_star_count[i];
        daPyMng_c::mPlayerType[i] = state->mPlayerType[i];
        daPyMng_c::m_yoshiColor[i] = state->m_yoshiColor[i];
    }

    // If the player resets from a non-Yoshi-riding state to a
    // Yoshi-riding state, the game is smart enough to start the Yoshi
    // drums automatically. But it's not smart enough to automatically
    // *stop* them in the opposite case, for some reason, so we have to
    // do it manually.
    if (!enable_yoshi_drums) {
        daPyMng_c::stopYoshiBGM();
    }
}


void restore_dCyuukan_c(STATE_dCyuukan_c *state) {
    dCyuukan_c *cyuukan = &dInfo_c::m_instance->cyuukan;
    cyuukan->playerSpawnPos = state->playerSpawnPos;
    cyuukan->curWorldLevelAreaAndEntrance = state->curWorldLevelAreaAndEntrance;
    cyuukan->isKinopioInChukan = state->isKinopioInChukan;
    cyuukan->starCoinStatus[0] = state->starCoinStatus[0];
    cyuukan->starCoinStatus[1] = state->starCoinStatus[1];
    cyuukan->starCoinStatus[2] = state->starCoinStatus[2];
}


void restore_dStageTimer_c(STATE_dStageTimer_c *state) {
    dStageTimer_c::m_instance->preciseTime = state->preciseTime;
}


void restore_dActorCreateMng_c(STATE_dActorCreateMng_c *state) {
    memcpy(dActorCreateMng_c::m_instance, state, sizeof(STATE_dActorCreateMng_c));

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


void restore_dFlagCtrl_c(STATE_dFlagCtrl_c *state) {
    memcpy(dFlagCtrl_c::m_instance, state, sizeof(STATE_dFlagCtrl_c));
}


void restore_zone_state_early(ZoneState *state) {
    restore_dScStage_c(&state->dScStage_c);
    restore_daPyMng_c(&state->daPyMng_c);
    restore_dCyuukan_c(&state->dCyuukan_c);
}


void restore_zone_state_late(ZoneState *state) {
    restore_dStageTimer_c(&state->dStageTimer_c);
    restore_dActorCreateMng_c(&state->dActorCreateMng_c);
    restore_dBgParameter_c();
    restore_dFlagCtrl_c(&state->dFlagCtrl_c);
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
