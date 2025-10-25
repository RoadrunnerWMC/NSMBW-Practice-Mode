#include "game.h"
#include "entrance_control.h"


NextGoto FakeEntrance = {0};
bool ForceFacingLeft = false;
u8 LastNonFakeEntrance;


// Track last non-fake entrance
kmBranchDefAsm(0x809261bc, 0x809261c0) {
    nofralloc

    cmpwi r4, FAKE_ENTRANCE_ID
    beq fake

    lis r6, LastNonFakeEntrance@ha
    addi r6, r6, LastNonFakeEntrance@l
    stb r4, 0(r6);

fake:
    stb r4, 0x1211(r3)  // original instruction
    blr
}


// Return the last entrance ID the player has spawned from, not
// counting the "fake" entrance ID.
u8 last_non_fake_entrance_id() {
    return LastNonFakeEntrance;
}


// Configure the fake entrance so that using it will warp you to an
// arbitrary position in an arbitrary area.
void configure_fake_entrance_to_pos(u8 dest_area, f32 dest_x, f32 dest_y) {
    u8 area_idx;
    if (dest_area == 0) {
        area_idx = dScStage_c::m_instance->curArea;
    } else {
        area_idx = dest_area - 1;
    }

    dCdFile_c *area_obj = &dCd_c::m_instance->areas[area_idx];

    mVec3_c v;
    v.x = dest_x;
    v.y = dest_y;
    v.z = 0.0f;  // getAreaNo() doesn't use this

    FakeEntrance.x = dest_x - 8.0f;
    FakeEntrance.y = -(dest_y + 16.0f);
    FakeEntrance.id = FAKE_ENTRANCE_ID;
    FakeEntrance.dest_area = dest_area;
    FakeEntrance.dest_id = FAKE_ENTRANCE_ID;
    FakeEntrance.zone_id = area_obj->getAreaNo(&v);
}


// Configure the fake entrance so that using it will warp you to some
// other entrance.
void configure_fake_entrance_to_other(u8 dest_area, u8 dest_id) {
    FakeEntrance.dest_area = dest_area;
    FakeEntrance.dest_id = dest_id;
}


// Force the player to spawn facing left the next time they spawn from
// any entrance.
void force_player_face_left_at_next_spawn() {
    ForceFacingLeft = true;
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
