#pragma once

#include "game.h"


struct PlayerSavestateLite {
    Vec pos;
    Vec lastPos;
    Vec moveDelta;
    Vec centerOffs;
    Vec speed;
    Vec speedMax;
    S16Vec angle;
    S16Vec moveAngle;
    f32 speedF;
    f32 accelF;
    f32 accelY;
    f32 accelFall;
    f32 accelX;
    u8 direction;
    int powerup;
    Vec camPos;
};


struct SavestateLite {
    u32 world_level_area_and_zone;
    PlayerSavestateLite player;
    dBgParameter_c bgParam;
    u32 timerPreciseTime;
};


void save_state(dAcPy_c *player, SavestateLite *state);
bool restore_state(dAcPy_c *player, SavestateLite *state);
