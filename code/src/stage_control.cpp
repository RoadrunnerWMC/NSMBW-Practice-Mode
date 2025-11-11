#include "game.h"
#include "stage_control.h"


void trigger_stage_reload() {
    if (dScStage_c::m_instance == NULL) {
        return;
    }

    // To make the transition as quick as possible, request a
    // "fade"-type fade (i.e. full-screen fade-to-black) with a time of
    // 0 (instant).
    // Note that the fade-out duration value persists after the
    // fade-out, and can affect other scenes such as crsin (aka level
    // banner), so it should be reset back to its default value of 30
    // later (we do that in a different hook).
    dFader_c::setFader(dFader_c::FADER_TYPE_FADE);
    dScene_c::setFadeOutFrame(0);

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


// Hook at the end of dScCrsin_c::create() to reset the fade-out length
// to its default/vanilla value of 30 frames
kmBranchDefCpp(0x8091f16c, NULL, u32, ) {
    dScene_c::setFadeOutFrame(30);
    return 1;
}
