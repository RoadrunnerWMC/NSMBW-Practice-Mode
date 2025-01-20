#include "game.h"
#include "savestate_lite.h"


void save_state(dAcPy_c *player, SavestateLite *state) {
    state->world_num = dScStage_c::m_instance->curWorld;
    state->level_num = dScStage_c::m_instance->curLevel;
    state->area_num = dScStage_c::m_instance->curArea;
    state->zone_num = dScStage_c::m_instance->curZone;
    state->player.pos = player->pos;
    state->player.lastPos = player->lastPos;
    state->player.moveDelta = player->moveDelta;
    state->player.centerOffs = player->centerOffs;
    state->player.speed = player->speed;
    state->player.speedMax = player->speedMax;
    state->player.angle = player->angle;
    state->player.moveAngle = player->moveAngle;
    state->player.speedF = player->speedF;
    state->player.accelF = player->accelF;
    state->player.accelY = player->accelY;
    state->player.accelFall = player->accelFall;
    state->player.accelX = player->accelX;
    state->player.direction = player->direction;
    state->player.powerup = player->powerup;
    state->player.camPos = player->camPos;

    memcpy(&state->bgParam, dBgParameter_c::ms_Instance_p, sizeof(dBgParameter_c));

    state->timerPreciseTime = dStageTimer_c::m_instance->preciseTime;

    // TODO: also save/restore all the random things that get reset in zone_reload.cpp
}


bool restore_state(dAcPy_c *player, SavestateLite *state) {
    if (state->world_num != dScStage_c::m_instance->curWorld
            || state->level_num != dScStage_c::m_instance->curLevel
            || state->area_num != dScStage_c::m_instance->curArea
            || state->zone_num != dScStage_c::m_instance->curZone) {
        return false;
    }

    player->pos = state->player.pos;
    player->lastPos = state->player.lastPos;
    player->moveDelta = state->player.moveDelta;
    player->centerOffs = state->player.centerOffs;
    player->speed = state->player.speed;
    player->speedMax = state->player.speedMax;
    player->angle = state->player.angle;
    player->moveAngle = state->player.moveAngle;
    player->speedF = state->player.speedF;
    player->accelF = state->player.accelF;
    player->accelY = state->player.accelY;
    player->accelFall = state->player.accelFall;
    player->accelX = state->player.accelX;
    player->direction = state->player.direction;
    player->setPowerupAlt((Powerup)(state->player.powerup));
    player->camPos = state->player.camPos;

    // if the player is still flashing from recent damage, cancel that
    player->flashTimer = 0;

    memcpy(dBgParameter_c::ms_Instance_p, &state->bgParam, sizeof(dBgParameter_c));

    dStageTimer_c::m_instance->preciseTime = state->timerPreciseTime;
    if (!dStageTimer_c::m_instance->isAmbush) {
        bool timeLessThan100 = ((state->timerPreciseTime + 0xfff) >> 12) < 101;
        if (dStageTimer_c::m_instance->timeLessThan100 && !timeLessThan100) {
            dStageTimer_c::m_instance->timeLessThan100 = false;
            // TODO: properly reset the music to the non-fast version
        }
    }

    dActorCreateMng_c::m_instance->doStuffForCurrentZone();

    return true;
}
