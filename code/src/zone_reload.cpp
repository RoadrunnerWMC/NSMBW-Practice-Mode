#include "game.h"
#include "zone_reload.h"


#define FAKE_ENTRANCE_ID 254

NextGoto FakeEntrance = {0};


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


void trigger_zone_reload(daPlBase_c *player, bool keep_current_pos) {
    if (keep_current_pos) {
        // TODO: also keep player facing-direction
        FakeEntrance.x = player->pos.x - 8.0f;
        FakeEntrance.y = -(player->pos.y + 16.0f);
        FakeEntrance.zone_id = dScStage_c::m_instance->curZone;
        FakeEntrance.dest_id = FAKE_ENTRANCE_ID;
    } else {
        FakeEntrance.dest_id = curEntranceNonFake;
    }

    player->useNextGotoBlock(FAKE_ENTRANCE_ID, 0, 0);
}


extern "C" NextGoto *getNextGotoP__9dCdFile_cFUc_PLUS_FOUR(u8 id);

kmBranchDefAsm(0x8008e3d0, NULL) {
    nofralloc

    cmpwi r4, FAKE_ENTRANCE_ID
    beq fake
    lwz r0, 0x90(r3)
    b getNextGotoP__9dCdFile_cFUc_PLUS_FOUR

fake:
    lis r3, FakeEntrance@h
    ori r3, r3, FakeEntrance@l
    blr
}
