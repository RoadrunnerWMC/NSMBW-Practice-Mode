#define FAKE_ENTRANCE_ID 254

u8 last_non_fake_entrance_id();
void configure_fake_entrance_to_pos(u8 dest_area, f32 dest_x, f32 dest_y);
void configure_fake_entrance_to_other(u8 dest_area, u8 dest_id);
void force_player_face_left_at_next_spawn();
