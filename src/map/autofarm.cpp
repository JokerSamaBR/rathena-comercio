// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
#include "autofarm.hpp"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#include <common/core.hpp>
#include <common/malloc.hpp>
#include <common/showmsg.hpp>
#include <common/sql.hpp>
#include <common/timer.hpp>

#include "chrif.hpp"
#include "itemdb.hpp"
#include "map.hpp"
#include "mob.hpp"
#include "pc.hpp"
#include "unit.hpp"

namespace autofarm {
namespace {

constexpr uint64_t kMaxDurationSeconds = 8ULL * 60ULL * 60ULL;
constexpr t_itemid kAutofarmModuleItem = 56330;
constexpr t_tick kTickInterval = 1000;

struct ActiveSession {
	SessionInfo info;
	std::vector<uint16_t> monster_ids;
};

std::unordered_map<uint32_t, ActiveSession> g_sessions;
std::unordered_map<uint32_t, std::vector<uint16_t>> g_pending_filters;
std::unordered_map<int32_t, uint32_t> g_clone_owners;

std::unordered_set<std::string> g_denied_maps;
int32_t g_timer = INVALID_TIMER;
std::atomic<uint32_t> g_reward_sequence{1};

static std::string trim(std::string value) {
	auto not_space = [](unsigned char c) { return !std::isspace(c); };
	value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
	value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
	return value;
}

static bool map_allowed(int16_t map_id) {
	if (map_id < 0)
		return false;
	if (map_getmapflag(map_id, MF_TOWN))
		return false;
	const char* name = map_getmapdata(map_id)->name;
	return g_denied_maps.find(name) == g_denied_maps.end();
}

static ActiveSession* find_by_char(uint32_t char_id) {
	auto it = g_sessions.find(char_id);
	return it == g_sessions.end() ? nullptr : &it->second;
}

static bool account_has_session(uint32_t account_id) {
	for (const auto& entry : g_sessions) {
		if (entry.second.info.account_id == account_id)
			return true;
	}
	return false;
}

static void load_denied_maps() {
	g_denied_maps.clear();
	std::ifstream file(std::string(conf_path) + "/autofarm.conf");
	if (!file.is_open()) {
		ShowInfo("Autofarm: no %s/autofarm.conf found; only town maps are blocked.\n", conf_path);
		return;
	}

	std::string line;
	while (std::getline(file, line)) {
		line = trim(line);
		if (line.empty() || line[0] == '#')
			continue;
		const size_t comment = line.find('#');
		if (comment != std::string::npos)
			line = trim(line.substr(0, comment));
		const size_t colon = line.find(':');
		if (colon == std::string::npos)
			continue;
		std::string key = trim(line.substr(0, colon));
		std::string value = trim(line.substr(colon + 1));
		if (key == "DenyMap" && !value.empty())
			g_denied_maps.insert(value);
	}
	ShowInfo("Autofarm: loaded %zu administrator-denied maps.\n", g_denied_maps.size());
}

static void erase_session(uint32_t char_id) {
	auto it = g_sessions.find(char_id);
	if (it == g_sessions.end())
		return;
	if (it->second.info.clone_id > 0)
		g_clone_owners.erase(it->second.info.clone_id);
	g_sessions.erase(it);
}

static void stop_internal(uint32_t char_id, const char* reason) {
	auto it = g_sessions.find(char_id);
	if (it == g_sessions.end())
		return;
	const int32_t clone_id = it->second.info.clone_id;
	if (clone_id > 0) {
		if (mob_data* clone = map_id2md(clone_id))
			unit_free(clone, CLR_OUTSIGHT);
	}
	ShowInfo("Autofarm: char %u stopped%s%s.\n", char_id,
		reason ? " (" : "", reason ? reason : "", reason ? ")" : "");
	erase_session(char_id);
}

static TIMER_FUNC(autofarm_timer) {
	const t_tick now = gettick();
	std::vector<uint32_t> to_stop;
	for (auto& entry : g_sessions) {
		ActiveSession& session = entry.second;
		if (session.info.clone_id <= 0 || map_id2md(session.info.clone_id) == nullptr) {
			to_stop.push_back(entry.first);
			continue;
		}
		if (static_cast<uint64_t>(time(nullptr)) >= session.info.expires_at) {
			to_stop.push_back(entry.first);
			continue;
		}
		mob_data* clone = map_id2md(session.info.clone_id);
		if (!map_allowed(clone->m)) {
			to_stop.push_back(entry.first);
			continue;
		}
				if (clone->target_id > 0) {
			block_list* target = map_id2bl(clone->target_id);
			if (target == nullptr || !target_allowed(clone, target))
				clone->target_id = 0;
		}

		// Run the cloned mob skill table explicitly. This is the mob-side
		// equivalent of the new4th autoattack skill loop.
		if (clone->target_id > 0)
			mobskill_use(clone, now, -1);

		// Keep an Autofarm clone patrolling instead of allowing it to remain
		// stationary after finishing a nearby target.
		if (clone->target_id == 0 && clone->ud.walktimer == INVALID_TIMER &&
			DIFF_TICK(now, clone->next_walktime) >= 0)
			mob_randomwalk(clone, now);

	}
	for (uint32_t char_id : to_stop)
		stop_internal(char_id, "expired, invalid, or destroyed");
	return kTickInterval;
}

} // namespace

void init() {
	load_denied_maps();
	add_timer_func_list(autofarm_timer, "autofarm_timer");
	g_timer = add_timer_interval(gettick() + kTickInterval, autofarm_timer, 0, 0, kTickInterval);
}

void final() {
	if (g_timer != INVALID_TIMER) {
		delete_timer(g_timer, autofarm_timer);
		g_timer = INVALID_TIMER;
	}
	std::vector<uint32_t> chars;
	for (const auto& entry : g_sessions)
		chars.push_back(entry.first);
	for (uint32_t char_id : chars)
		stop_internal(char_id, "server shutdown");
	g_clone_owners.clear();
	g_sessions.clear();
}

bool start(map_session_data* owner, const std::vector<uint16_t>& monster_ids) {
	if (owner == nullptr)
		return false;
		if (owner->status.char_id == 0 || !map_allowed(owner->m))
		return false;
	// The rental module is the license that unlocks Autofarm.
	if (pc_search_inventory(owner, kAutofarmModuleItem) < 0)
		return false;
	if (find_by_char(owner->status.char_id) != nullptr || account_has_session(owner->status.account_id))

		return false;

	// The native clone copies the player's status, view and skill tree and uses
	// the normal monster AI. It has no inventory and therefore cannot duplicate
	// the owner's items.
	const uint32_t duration_ms = static_cast<uint32_t>(kMaxDurationSeconds * 1000ULL);
		std::vector<uint16_t> selected_monsters = monster_ids;
	if (selected_monsters.empty()) {
		auto pending = g_pending_filters.find(owner->status.char_id);
		if (pending != g_pending_filters.end())
			selected_monsters = pending->second;
	}

	const int32_t clone_id = mob_clone_spawn(owner, owner->m, owner->x, owner->y,
		"", 0, MD_NONE, 1, duration_ms);
	if (clone_id <= 0)
		return false;

	mob_data* clone = map_id2md(clone_id);
	if (clone == nullptr)
		return false;

	// A player snapshot does not carry the aggressive mob flags required by the
	// native hard-AI loop. Enable the normal mob search, chase and walk states.
	clone->status.mode = static_cast<enum e_mode>(clone->status.mode |
		MD_AGGRESSIVE | MD_CANATTACK | MD_CANMOVE | MD_CHANGECHASE);
		clone->state.aggressive = 1;
		clone->state.skillstate = MSS_IDLE;
		clone->last_skillcheck = gettick() - MOB_SKILL_INTERVAL;
		clone->next_walktime = gettick();

	ActiveSession session;

	session.info.active = true;
	session.info.account_id = owner->status.account_id;
	session.info.char_id = owner->status.char_id;
	session.info.clone_id = clone_id;
	session.info.map_id = owner->m;
	session.info.started_at = static_cast<uint64_t>(time(nullptr));
	session.info.expires_at = session.info.started_at + kMaxDurationSeconds;
		session.monster_ids = std::move(selected_monsters);

	g_clone_owners[clone_id] = owner->status.char_id;
	g_sessions.emplace(owner->status.char_id, std::move(session));
	ShowInfo("Autofarm: started char %u with clone %d on map %s for 8 hours.\n",
		owner->status.char_id, clone_id, map_getmapdata(owner->m)->name);
	return true;
}

bool stop(uint32_t char_id, const char* reason) {
	if (find_by_char(char_id) == nullptr)
		return false;
	stop_internal(char_id, reason ? reason : "command");
	return true;
}

bool status(uint32_t char_id, SessionInfo& out) {
	ActiveSession* session = find_by_char(char_id);
	if (session == nullptr)
		return false;
	if (session->info.clone_id <= 0 || map_id2md(session->info.clone_id) == nullptr) {
		stop_internal(char_id, "clone no longer exists");
		return false;
	}
	out = session->info;
	return true;
}

bool add_monster_filter(uint32_t char_id, const char* name) {
	if (name == nullptr || !*name)
		return false;
	const uint16_t mob_id = mobdb_searchname(name);
	if (mob_id == 0)
		return false;

	std::vector<uint16_t>* filter = nullptr;
	if (ActiveSession* session = find_by_char(char_id))
		filter = &session->monster_ids;
	else
		filter = &g_pending_filters[char_id];

	if (std::find(filter->begin(), filter->end(), mob_id) == filter->end())
		filter->push_back(mob_id);
	return true;
}

bool remove_monster_filter(uint32_t char_id, const char* name) {
	if (name == nullptr || !*name)
		return false;
	const uint16_t mob_id = mobdb_searchname(name);
	if (mob_id == 0)
		return false;

	std::vector<uint16_t>* filter = nullptr;
	if (ActiveSession* session = find_by_char(char_id))
		filter = &session->monster_ids;
	else {
		auto pending = g_pending_filters.find(char_id);
		if (pending == g_pending_filters.end())
			return false;
		filter = &pending->second;
	}

	auto it = std::find(filter->begin(), filter->end(), mob_id);
	if (it == filter->end())
		return false;
	filter->erase(it);
	return true;
}

bool clear_monster_filter(uint32_t char_id) {
	if (ActiveSession* session = find_by_char(char_id)) {
		session->monster_ids.clear();
		return true;
	}
	return g_pending_filters.erase(char_id) > 0;
}

std::vector<uint16_t> get_monster_filter(uint32_t char_id) {
	ActiveSession* session = find_by_char(char_id);
	return session == nullptr ? std::vector<uint16_t>() : session->monster_ids;
}

bool target_allowed(const mob_data* attacker, const block_list* target) {
	if (attacker == nullptr || target == nullptr)
		return false;
	auto owner_it = g_clone_owners.find(attacker->id);
	if (owner_it == g_clone_owners.end())
		return true; // Normal mobs keep their original behavior.
	if (target->type != BL_MOB)
		return false;
	const ActiveSession* session = find_by_char(owner_it->second);
	if (session == nullptr)
		return false;
	if (session->monster_ids.empty())
		return true;
	const mob_data* target_mob = reinterpret_cast<const mob_data*>(target);
	return std::find(session->monster_ids.begin(), session->monster_ids.end(), target_mob->mob_id) != session->monster_ids.end();
}

uint32_t owner_char_id(const block_list* attacker) {
	if (attacker == nullptr)
		return 0;
	auto it = g_clone_owners.find(attacker->id);
	return it == g_clone_owners.end() ? 0 : it->second;
}

bool has_active_session(uint32_t char_id) {
	return find_by_char(char_id) != nullptr;
}

void record_kill(uint32_t char_id) {
	if (ActiveSession* session = find_by_char(char_id))
		session->info.kills++;
}

bool consume_drop(uint32_t char_id, const item& drop, int32_t amount) {
	ActiveSession* session = find_by_char(char_id);
	if (session == nullptr || drop.nameid == 0 || amount <= 0)
		return false;
	item reward = drop;
	reward.amount = static_cast<int16_t>(std::min<int32_t>(amount, MAX_AMOUNT));
	const auto item_data = itemdb_search(reward.nameid);
	if (reward.unique_id == 0 && item_data != nullptr && item_data->flag.guid) {
		const uint32_t seq = g_reward_sequence.fetch_add(1);
		reward.unique_id = (static_cast<uint64_t>(char_id) << 32) | (0x80000000ULL | seq);
	}
		// Store the reward in the dedicated Autofarm deposit. The NPC will
	// deliver it later, keeping the normal Kafra storage untouched.
	if (SQL_ERROR == Sql_Query(mmysql_handle,
		"INSERT INTO `autofarm_rewards` (`account_id`,`char_id`,`nameid`,`amount`,`identify`,`refine`,`attribute`,`card0`,`card1`,`card2`,`card3`) VALUES (%u,%u,%u,%d,%u,%u,%u,%u,%u,%u,%u)",
		session->info.account_id, session->info.char_id, reward.nameid, reward.amount,
		reward.identify, reward.refine, reward.attribute,
		reward.card[0], reward.card[1], reward.card[2], reward.card[3])) {
		Sql_ShowDebug(mmysql_handle);
		return false;
	}
	session->info.rewards += reward.amount;
	return true;

}

void owner_logout(const map_session_data* owner) {
	// The clone has master_id == 0 and is therefore not part of the normal
	// slave cleanup. The session remains in g_sessions until expiry or /off.
	(void)owner;
}

} // namespace autofarm
