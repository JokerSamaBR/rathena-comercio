// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
#ifndef AUTOFARM_HPP
#define AUTOFARM_HPP

#include <cstdint>
#include <string>
#include <vector>

class map_session_data;
struct mob_data;
struct block_list;
struct item;
	namespace autofarm {

struct SessionInfo {
	bool active = false;
	uint32_t account_id = 0;
	uint32_t char_id = 0;
	int32_t clone_id = 0;
	int16_t map_id = -1;
	uint64_t started_at = 0;
	uint64_t expires_at = 0;
	uint32_t kills = 0;
	uint64_t rewards = 0;
};

void init();
void final();

bool start(map_session_data* owner, const std::vector<uint16_t>& monster_ids);
bool stop(uint32_t char_id, const char* reason = nullptr);
bool status(uint32_t char_id, SessionInfo& out);

bool add_monster_filter(uint32_t char_id, const char* name);
bool remove_monster_filter(uint32_t char_id, const char* name);
bool clear_monster_filter(uint32_t char_id);
std::vector<uint16_t> get_monster_filter(uint32_t char_id);

// Called by the mob AI before selecting a target. Normal mobs are unaffected.
bool target_allowed(const mob_data* attacker, const block_list* target);
uint32_t owner_char_id(const block_list* attacker);
bool has_active_session(uint32_t char_id);
void record_kill(uint32_t char_id);

// Called by the mob drop pipeline. Returns true when the drop was consumed by Autofarm.
bool consume_drop(uint32_t owner_char_id, const item& drop, int32_t amount);

// Called by map-server logout before the player object is freed.
void owner_logout(const map_session_data* owner);

} // namespace autofarm

#endif // AUTOFARM_HPP
