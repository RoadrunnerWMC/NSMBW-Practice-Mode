#pragma once

#include <kamek.h>


// unofficial enum name
enum Powerup {
    POWERUP_SMALL = 0,
    POWERUP_SUPER = 1,
    POWERUP_FIRE = 2,
    POWERUP_MINI = 3,
    POWERUP_PROPELLER = 4,
    POWERUP_PENGUIN = 5,
    POWERUP_ICE = 6,
};


// unofficial enum name
enum ControlStyle {
    SIDEWAYS = 0,
    NUNCHUK = 1,
};


class dAcPyKey_c {
public:
    // For the "held"/"pressed" fields:
    // bit        sideways     nunchuk
    // 0000_0001  d-pad down   d-pad down, nunchuk down
    // 0000_0002  d-pad up     d-pad up, nunhuk up
    // 0000_0004  d-pad right  d-pad right, nunchuk right
    // 0000_0008  d-pad left   d-pad left, nunchuk left
    // 0000_0010  plus         plus
    // 0000_0100  2            A
    // 0000_0200  1            B, nunchuk Z
    // 0000_0400  B            B
    // 0000_0800  A            1, 2, nunchuk C
    // 0000_1000  minus        minus
    // 0000_2000               nunchuk Z
    // 0000_8000  home         home

    /* 0x00 */ s32 remoconID;
    /* 0x04 */ u16 heldButtons;
    /* 0x06 */ u16 nowPressed;
    /* 0x08 */ u16 lastHeldButtons;
    /* 0x0a */ u16 lastNowPressed;
    /* 0x0c */ u16 applyToHeldButtonsWithFlag7_permanent;
    /* 0x0e */ u16 applyToHeldButtonsWithFlag7_transient;
    /* 0x10 */ u16 forcedShakeValue;
    /* 0x12 */ u16 flags;
        // flag 0: No Input
        // flag 3: force Two on
        // flag 4: force Two off
        // flag 6: Nothing But Holding Right
        // flag 7: Force Specific Values
        // flag 8: No Shaking
    /* 0x14 */ u8 downHeldCounter;
    /* 0x15 */ u8 dontResetGPValuesNextTick;
    /* 0x16 */ u8 currentlyHoldingDown;
    /* 0x17 */ bool shake_jump;
    /* 0x18 */ bool shaking_18;
    /* 0x19 */ bool wasActionExecuted;
    /* 0x1a */ bool unk1A;
    /* 0x1b */ bool unk1B;
    /* 0x1c */ int countdownAfterFlag6Deactivated;
    /* 0x20 */ u8 unk20;
    /* 0x21 */ u8 unk21;
    /* 0x22 */ u8 unk22;
    /* 0x23 */ u8 unk23;
    /* 0x24 */ u8 rollingHistoryOfTwo[0xa0];  // dummy
    /* 0xc4 */ u8 rollingHistoryOfTwoModifiedByFlags[0xa0];  // dummy

    void update();
};


class daPlBase_c;


class daPlBase_c_vtable {
public:
    /* 0x000 */ u8 pad1[0x3d4];
    /* 0x3d4 */ u32 (*isStar)(daPlBase_c *this_);
    /* 0x3d8 */ void (*startStar)(daPlBase_c *this_, int behavior, int timer);
    /* 0x3dc */ void (*endStar)(daPlBase_c *this_);
};


class daPlBase_c {
public:
    void useNextGotoBlock(u32 exitID, u32 delay, u32 transition);  // unofficial method name
    void playSound(unsigned int id, long unk);  // unofficial method name

    /* 0x000 */ u8 pad1[0x60];

    /* 0x060 */ daPlBase_c_vtable *vtable;

    /* 0x064 */ u8 pad2[0x48];

    /* 0x0ac */ Vec pos;
    /* 0x0b8 */ Vec lastPos;
    /* 0x0c4 */ Vec moveDelta;
    /* 0x0d0 */ Vec centerOffs;
    /* 0x0dc */ Vec size;
    /* 0x0e8 */ Vec speed;
    /* 0x0f4 */ Vec speedMax;
    /* 0x100 */ S16Vec angle;
    /* 0x106 */ S16Vec moveAngle;
    /* 0x10c */ f32 speedF;
    /* 0x110 */ f32 accelF;
    /* 0x114 */ f32 accelY;
    /* 0x118 */ f32 accelFall;
    /* 0x11c */ f32 accelX;

    /* 0x120 */ u8 pad3[0x228];

    /* 0x348 */ u8 direction;

    /* 0x349 */ u8 pad4[0xb5b];

    /* 0xea4 */ dAcPyKey_c input;

    /* 0x1008 */ u8 pad5[0x68];

    /* 0x1070 */ u32 starTimer;
    /* 0x1074 */ u32 flashTimer;

    /* 0x1078 */ u8 pad6[0x18];

    /* 0x1090 */ int powerup;

    // technically in dAcPy_c probably
    /* 0x1094 */ u8 pad7[0x498];
    /* 0x152c */ Vec camPos;
};


class dAcPy_c : public daPlBase_c {
public:
    void setPowerupAlt(Powerup powerup);  // unofficial method name
};


class daPyMng_c {
public:
    static u32 mRest[4];
    static u32 mCoin[4];
    static u32 mNum;
    static u32 mScore;
    static u32 mActPlayerInfo;

    static dAcPy_c *getPlayer(int num);
};


class dBgParameter_c {
public:
    /* 0x00 */ void *vtable;
    /* 0x04 */ u16 *tileBuffers[12];
    /* 0x34 */ f32 unk34;
    /* 0x38 */ f32 screenLeft;
    /* 0x3c */ f32 screenTop;
    /* 0x40 */ f32 screenWidth;
    /* 0x44 */ f32 screenHeight;
    /* 0x48 */ Vec2 screenCentre;
    /* 0x50 */ Vec2 screenCentre_calcedByBgGm;
    /* 0x58 */ Vec2 base_pos;
    /* 0x60 */ f32 org_x;
    /* 0x64 */ f32 unk64;
    /* 0x68 */ f32 unk68;
    /* 0x6c */ f32 unk6C;
    /* 0x70 */ f32 unk70;
    /* 0x74 */ f32 unk74;
    /* 0x78 */ Vec2 offset;
    /* 0x80 */ u8 movedDirectionSinceLastTickX;
    /* 0x81 */ u8 movedDirectionSinceLastTickY;
    /* 0x82 */ u8 unk82;
    /* 0x83 */ u8 unk83;
    /* 0x84 */ u32 p_switch_state;
    /* 0x88 */ int p_switch_timers[3];
    /* 0x94 */ void *bgHeap;

    static dBgParameter_c *ms_Instance_p;
};


class dBg_c {
public:
    u8 pad1[0x8fe18];
    /* 0x8fe18 */ f32 unk8FE18;
    /* 0x8fe1c */ u8 pad2[0x28];
    /* 0x8fe44 */ f32 unk8FE44;
    /* 0x8fe48 */ u8 pad3[0x4];
    /* 0x8fe4c */ f32 unk8FE4C;
    /* 0x8fe50 */ f32 unk8FE50;
    /* 0x8fe54 */ u8 pad4[0x10];
    /* 0x8fe64 */ f32 zoneLeft;
    /* 0x8fe68 */ f32 zoneRight;
    /* 0x8fe6c */ f32 zoneTop;
    /* 0x8fe70 */ f32 zoneBottom;
    /* 0x8fe74 */ u8 pad5[0x14];
    /* 0x8fe88 */ f32 autoscroll_left;
    /* 0x8fe8c */ f32 autoscroll_top;
    /* 0x8fe90 */ f32 previousAutoscrollLeft;
    /* 0x8fe94 */ f32 previousAutoscrollTop;
    /* 0x8fe98 */ u8 pad6[0x10];
    /* 0x8fea8 */ f32 previousScreenLeft;
    /* 0x8feac */ f32 previousScreenTop;
    /* 0x8feb0 */ u8 pad7[0x16c];
    /* 0x9001c */ f32 cameraPanAmount;
    /* 0x90020 */ u8 pad8[0x4];
    /* 0x90024 */ bool isNotFollowingAnyPlayer;
    /* 0x90025 */ u8 pad9[0x0b];
    /* 0x90030 */ f32 zoneLeft_copy1;
    /* 0x90034 */ f32 zoneRight_copy1;
    /* 0x90038 */ f32 zoneTop_copy1;
    /* 0x9003c */ f32 zoneBottom_copy1;
    /* 0x90040 */ f32 zoneLeftSingle;
    /* 0x90044 */ f32 zoneRightSingle;
    /* 0x90048 */ f32 zoneTopSingle;
    /* 0x9004c */ f32 zoneBottomSingle;
    /* 0x90050 */ f32 zoneLeftMulti;
    /* 0x90054 */ f32 zoneRightMulti;
    /* 0x90058 */ f32 zoneTopMulti;
    /* 0x9005c */ f32 zoneBottomMulti;
    /* 0x90060 */ u8 pad10[0xbc];
    /* 0x9011c */ f32 field_9011C;
    /* 0x90120 */ f32 field_90120;
    /* 0x90124 */ u8 pad11[0x8b4];
    /* 0x909d8 */ f32 x1_calc_from_entrance_c;
    /* 0x909dc */ f32 y1_calc_from_entrance_c;
    /* 0x909e0 */ f32 x2_calc_from_entrance_c;
    /* 0x909e4 */ f32 y2_calc_from_entrance_c;
    /* 0x909e8 */ u8 pad12[0x24];
    /* 0x90a0c */ f32 x1_calc_from_entrance;
    /* 0x90a10 */ f32 y1_calc_from_entrance;
    /* 0x90a14 */ f32 x2_calc_from_entrance;
    /* 0x90a18 */ f32 y2_calc_from_entrance;
    // length: 0x90aac

    static dBg_c *m_bg_p;
};


struct NextGoto {
    /* 0x00 */ u16 x;
    /* 0x02 */ u16 y;
    /* 0x04 */ u16 _4;
    /* 0x06 */ u16 _6;
    /* 0x08 */ u8 id;
    /* 0x09 */ u8 dest_area;
    /* 0x0a */ u8 dest_id;
    /* 0x0b */ u8 type;
    /* 0x0c */ u8 _C;
    /* 0x0d */ u8 zone_id;
    /* 0x0e */ u8 layer;
    /* 0x0f */ u8 direct_pipe_path_id;
    /* 0x10 */ u16 flags;
    /* 0x12 */ bool leave_stage;
    /* 0x13 */ u8 _13;
};


class dCamera_c {
public:
    u8 pad[0x384];
};


class dScStage_c {
public:
    u8 pad[0x120c];
    // Using a union so we can do some optimizations in certain places
    union {
        struct {
            /* 0x120c */ u8 curWorld;
            /* 0x120d */ u8 curLevel;
            /* 0x120e */ u8 curArea;
            /* 0x120f */ u8 curZone;
        };
        struct {
            /* 0x120c */ u16 curWorldAndLevel;
            /* 0x120e */ u16 curAreaAndZone;
        };
        /* 0x120c */ u32 curWorldLevelAreaAndZone;
    };
    /* 0x1210 */ u8 curLayer;
    /* 0x1211 */ u8 curEntrance;

    static dCamera_c *m_camera[1];
    static dScStage_c *m_instance;
    static u32 mCollectionCoin[3];

    static bool getCourseIn();
    void restoreOldPlayerInfo();
};


class dNext_c {
public:
    void initGoto(u8, u8, u32);

    static dNext_c *m_instance;
};

class dActorCreateMng_c {
public:
    void ActorCreateInfoClear(void);
    void MapActorInital_next(void);

    static dActorCreateMng_c *m_instance;

    /* 0x000 */ u32 counters[4];
    /* 0x010 */ u16 storedShorts[1000];
    /* 0x7e0 */ u8 storedBytes[1000];
    /* 0xbc8 */ u16 unkBC8;
    /* 0xbca */ bool isEndingDemo;
};

class LytTextBox_c {
public:
    void setText(const wchar_t*, long, ...);
};

class dGameDisplay_c {
public:
    /* 0x000 */ u8 pad1[0x3e0];
    /* 0x3e0 */ int timer;
    /* 0x3e4 */ u8 pad2[0xfc];
    /* 0x4e0 */ LytTextBox_c *timerBox;
};

class dStageTimer_c {
public:
    /* 0x0 */ void *vtable;
    /* 0x4 */ u32 preciseTime;
    /* 0x8 */ s16 initialTime;
    /* 0xa */ bool isAmbush;
    /* 0xb */ bool timeLessThan100;
    /* 0xc */ bool isPaused;

    static dStageTimer_c *m_instance;
};


namespace EGG {
    class CoreController {
    public:
        /* 0x00 */ u8 pad1[0x18];
        /* 0x18 */ u32 hold;  // really in KPad
    };
}



namespace mPad {
    class CH_e {
    public:
        /* 0x00 */ u8 MPAD_0;
        /* 0x01 */ u8 MPAD_1;
        /* 0x02 */ u8 MPAD_2;
        /* 0x03 */ u8 MPAD_3;
    };

    static EGG::CoreController *g_core[4];
};


class dGameKeyCore_c {
public:
    /* 0x00 */ void* vtable;
    ///* 0x04 */ mPad::CH_e id;
    /* 0x04 */ u32 id;
    /* 0x08 */ ControlStyle controllerType;
    /* 0x0c */ u32 hold;
    /* 0x10 */ u32 prev_hold;
    /* 0x14 */ u32 held_buttons;
    /* 0x18 */ u32 prev_held_buttons;
    /* 0x1c */ u32 newly_pressed;
    /* 0x20 */ u32 field_20;
    /* 0x24 */ u32 field_24;
    /* 0x28 */ u32 field_28;
    /* 0x2c */ u32 field_2C;
    /* 0x30 */ u32 field_30;
    /* 0x34 */ Vec acc;
    /* 0x40 */ Vec prev_acc;
    /* 0x4c */ Vec2 acc_vert_x;
    /* 0x54 */ Vec2 prev_acc_vert_x;
    /* 0x5c */ Vec2 acc_vert_y;
    /* 0x64 */ Vec2 acc_vert_z;
    /* 0x74 */ Vec2 angle;
    /* 0x7c */ Vec2 last_angle;
    /* 0x84 */ f32 move_dist;
    /* 0x88 */ f32 prev_move_dist;
    /* 0x8c */ bool is_shaking;
    /* 0x8d */ u8 field_8D;
    /* 0x8e */ u16 tiltAmount;
    /* 0x90 */ s8 shake_timer1;
    /* 0x91 */ s8 shake_timer2;
    /* 0x92 */ u8 shake_timer3;
    /* 0x93 */ u8 field_93;
};


class dGameKey_c {
public:
    /* 0x00 */ void *vtable;
    /* 0x04 */ dGameKeyCore_c *remocons[4];

    static dGameKey_c *m_instance;
};


class dFlagCtrl_c {
public:
    void clearAllFlagData();

    static dFlagCtrl_c *m_instance;
};


class dCyuukan_c {
public:
    void clear();
};


class dInfo_c {
public:
    typedef enum hint_movie_type_e {
        HINT_MOVIE_TYPE_SUPER_SKILLS = 0,
        HINT_MOVIE_TYPE_1UP,
        HINT_MOVIE_TYPE_STAR_COIN,
        HINT_MOVIE_TYPE_SECRET_EXIT,
    } hint_movie_type_e;

    typedef enum screen_type_e {
        SCREEN_TYPE_NORMAL = 0,
        SCREEN_TYPE_SUPER_GUIDE,
        SCREEN_TYPE_TITLE,
        SCREEN_TYPE_TITLE_REPLAY,
        SCREEN_TYPE_HINT_MOVIE,
    } screen_type_e;

    typedef struct StartGameInfo_s {
        /* 0x00 */ u32 replay_duration;
        /* 0x04 */ u8 hint_movie_type;  // hint_movie_type_e
        /* 0x05 */ u8 entrance;
        /* 0x06 */ u8 area;
        /* 0x07 */ bool is_replay;
        /* 0x08 */ u32 screen_type;  // screen_type_e
        /* 0x0c */ u8 world_1;
        /* 0x0d */ u8 level_1;
        /* 0x0e */ u8 world_2;
        /* 0x0f */ u8 level_2;
    } StartGameInfo_s;

    void startGame(const dInfo_c::StartGameInfo_s&);

    static dInfo_c *m_instance;
    static u32 mGameFlag;

    /* 0x000 */ u8 pad[0x008];
    /* 0x008 */ dCyuukan_c cyuukan;
};

// unofficial function name
void ReturnToAnotherSceneAfterLevel(u32, u32, u32, u32);


class dFader_c {
public:
    typedef enum fader_type_e {
        FADER_TYPE_FADE = 0,
        FADER_TYPE_CIRCLE_1,
        FADER_TYPE_BOWSER,
        FADER_TYPE_WAVY,
        FADER_TYPE_MARIO_HEAD,
        FADER_TYPE_CIRCLE_5,
    } fader_type_e;

    static bool setFader(fader_type_e type);
};


class dScene_c {
public:
    static void setNextScene(u16, unsigned long, bool);
};


class dAudio {
public:
    static void hashname_a2bd17ff_6bcc38cc(s32);
};
