// Custom Season / Custom Rate Engine
// Revised port for the current rAthena tree.

#include "customrate.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>

#include <common/malloc.hpp>
#include <common/nullpo.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/strlib.hpp>
#include <common/utils.hpp>

#include "battle.hpp"
#include "clif.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "map.hpp"
#include "npc.hpp"
#include "pc.hpp"
#include "script.hpp"
#include "status.hpp"

using namespace rathena;

static constexpr const char* CUSTOMRATE_ID_VAR = "CUSTOM_RATE2_ID";
static constexpr const char* CUSTOMRATE_STATE_VAR = "CUSTOM_RATE2_STATE";
static constexpr const char* CUSTOMRATE_TIME_VAR = "CUSTOM_RATE2_TIME";
static constexpr const char* CUSTOMRATE_REWARD_PREFIX = "CUSTOM_RATE2_REWARD_";
static constexpr const char* CUSTOMRATE_FIRST_LOGIN_VAR = "SEASON_FIRST_LOGIN_DONE";
static constexpr const char* CUSTOMRATE_FIRST_LOGIN_EVENT = "SeasonRateLogin::OnFirstLogin";
// 0x1000 is unused by the native bonus-script flags and is reserved for SeasonEngine.
static constexpr uint32 CUSTOMRATE_BONUS_FLAG = 0x1000;
// bonus_script treats INFINITE_TICK (-1) as an already-expired timer. Use a long finite duration;
// the entry is reapplied whenever the character is loaded while the rate remains active.
static constexpr t_tick CUSTOMRATE_BONUS_DURATION = static_cast<t_tick>(3650LL * 24 * 60 * 60 * 1000);

CustomRateDatabase customrate_db;

const std::string CustomRateDatabase::getDefaultLocation() {
	return std::string(db_path) + "/custom/customrate_db.yml";
}

uint64 CustomRateDatabase::parseBodyNode(const ryml::NodeRef& node) {
	uint32 id = 0;
	if (!this->asUInt32(node, "Id", id) || id == 0)
		return 0;

	std::shared_ptr<s_customrate_db> rate = this->find(id);
	const bool exists = rate != nullptr;
	auto read_int16 = [&](const char* key, int32& value, int32 minimum, int32 maximum) {
		int16 parsed = 0;
		if (!this->asInt16(node, key, parsed))
			return false;
		if (parsed < minimum || parsed > maximum) {
			this->invalidWarning(node, "Value for %s must be between %d and %d.\n", key, minimum, maximum);
			return false;
		}
		value = parsed;
		return true;
	};
	if (!exists) {
		if (!this->nodesExist(node, { "Name" }))
			return 0;
		rate = std::make_shared<s_customrate_db>();
		rate->id = id;
	}

	if (this->nodeExists(node, "Name")) {
		std::string name;
		if (!this->asString(node, "Name", name))
			return 0;
		name.resize(NAME_LENGTH);
		rate->name = name;
	}

	if (this->nodeExists(node, "BaseRate") && !read_int16("BaseRate", rate->base_rate, 1, 1000))
		return 0;
	if (this->nodeExists(node, "JobRate") && !read_int16("JobRate", rate->job_rate, 1, 1000))
		return 0;
	if (this->nodeExists(node, "DropRate") && !read_int16("DropRate", rate->drop_rate, 1, 1000))
		return 0;
	if (this->nodeExists(node, "Effect") && !read_int16("Effect", rate->effect, 0, EFST_MAX - 1))
		return 0;
	if (this->nodeExists(node, "TitleId") && !read_int16("TitleId", rate->title_id, 0, INT16_MAX))
		return 0;

	if (this->nodeExists(node, "TimeLimit")) {
		std::string time_limit;
		if (!this->asString(node, "TimeLimit", time_limit))
			return 0;
		if (time_limit.empty() || time_limit == "0") {
			rate->duration = 0;
		} else if (time_limit.find('+') != std::string::npos) {
			rate->duration = static_cast<time_t>(solve_time(const_cast<char*>(time_limit.c_str())));
			if (rate->duration < 0) {
				this->invalidWarning(node["TimeLimit"], "Invalid TimeLimit %s.\n", time_limit.c_str());
				return 0;
			}
		}
	}

	if (this->nodeExists(node, "Script")) {
		std::string source;
		if (!this->asString(node, "Script", source))
			return 0;
		if (rate->script)
			script_free_code(rate->script);
		rate->script = source.empty() ? nullptr : parse_script(source.c_str(), this->getCurrentFile().c_str(), id, SCRIPT_IGNORE_EXTERNAL_BRACKETS);
	}

	if (this->nodeExists(node, "ScriptEnd")) {
		std::string source;
		if (!this->asString(node, "ScriptEnd", source))
			return 0;
		if (rate->script_end)
			script_free_code(rate->script_end);
		rate->script_end = source.empty() ? nullptr : parse_script(source.c_str(), this->getCurrentFile().c_str(), id, SCRIPT_IGNORE_EXTERNAL_BRACKETS);
	}

	if (this->nodeExists(node, "FirstTimeReward")) {
		rate->first_rewards.clear();
		const ryml::NodeRef rewards = node["FirstTimeReward"];
		if (!rewards.is_seq()) {
			this->invalidWarning(rewards, "FirstTimeReward must be a sequence.\n");
			return 0;
		}
		for (const ryml::NodeRef& reward_node : rewards) {
			uint32 item_id = 0;
			int16 amount = 1;
			if (!this->asUInt32(reward_node, "ItemId", item_id) || !item_db.exists(item_id)) {
				this->invalidWarning(reward_node, "FirstTimeReward contains an unknown ItemId.\n");
				continue;
			}
			if (this->nodeExists(reward_node, "Amount") && !this->asInt16(reward_node, "Amount", amount))
				return 0;
			if (amount < 1)
				amount = 1;
			rate->first_rewards.push_back({ static_cast<t_itemid>(item_id), amount });
		}
	}

	if (this->nodeExists(node, "BonusScript")) {
		std::string source;
		if (!this->asString(node, "BonusScript", source))
			return 0;
		rate->bonus_script = source;
	}

	if (!exists)
		this->put(id, rate);
	return 1;
}

std::shared_ptr<s_customrate_db> customrate_search(uint32 id) {
	return customrate_db.find(id);
}

const s_customrate_data* customrate_get_state(const map_session_data* sd) {
	return sd ? &sd->customrate : nullptr;
}

int32 customrate_get_id(const map_session_data* sd) {
	return sd ? sd->customrate.id : 0;
}

int32 customrate_get_state_value(const map_session_data* sd) {
	return sd ? sd->customrate.state : CUSTOM_RATE_INACTIVE;
}

time_t customrate_get_time(const map_session_data* sd) {
	return sd ? sd->customrate.expires_at : 0;
}

static void customrate_message(map_session_data* sd, const char* message) {
	if (sd && message && sd->fd > 0 && session_isActive(sd->fd))
		clif_displaymessage(sd->fd, message);
}

TIMER_FUNC(customrate_first_login_timer);
TIMER_FUNC(customrate_script_timer);

static void customrate_cancel_first_login_timer(map_session_data* sd) {
	if (!sd)
		return;
	if (sd->customrate.first_login_timer != INVALID_TIMER) {
		delete_timer(sd->customrate.first_login_timer, customrate_first_login_timer);
		sd->customrate.first_login_timer = INVALID_TIMER;
	}
}

static void customrate_schedule_first_login(map_session_data* sd) {
	if (!sd || !battle_config.customrate_enable)
		return;
	if (sd->customrate.state == CUSTOM_RATE_ACTIVE || pc_readglobalreg(sd, add_str(CUSTOMRATE_FIRST_LOGIN_VAR)) != 0)
		return;
	customrate_cancel_first_login_timer(sd);
	sd->customrate.first_login_timer = add_timer(gettick() + 2500, customrate_first_login_timer, sd->id, 0);
}

TIMER_FUNC(customrate_first_login_timer) {
	map_session_data* sd = map_id2sd(id);
	if (!sd)
		return 1;
	if (sd->customrate.first_login_timer != tid)
		return 0;
	sd->customrate.first_login_timer = INVALID_TIMER;
	if (sd->customrate.state == CUSTOM_RATE_ACTIVE || pc_readglobalreg(sd, add_str(CUSTOMRATE_FIRST_LOGIN_VAR)) != 0)
		return 0;
	npc_event(sd, CUSTOMRATE_FIRST_LOGIN_EVENT, 0);
	return 0;
}

static void customrate_save_state(map_session_data* sd) {
	nullpo_retv(sd);
	pc_setglobalreg(sd, add_str(CUSTOMRATE_ID_VAR), sd->customrate.id);
	pc_setglobalreg(sd, add_str(CUSTOMRATE_STATE_VAR), sd->customrate.state);
	pc_setglobalreg(sd, add_str(CUSTOMRATE_TIME_VAR), static_cast<int64>(sd->customrate.expires_at));
}

static void customrate_cancel_timer(map_session_data* sd) {
	if (!sd)
		return;
	if (sd->customrate.timer != INVALID_TIMER) {
		delete_timer(sd->customrate.timer, customrate_check_timer);
		sd->customrate.timer = INVALID_TIMER;
	}
}

static void customrate_schedule_timer(map_session_data* sd) {
	if (!sd || sd->customrate.expires_at <= 0 || sd->customrate.state != CUSTOM_RATE_ACTIVE)
		return;
	customrate_cancel_timer(sd);
	sd->customrate.timer = add_timer(gettick() + 1000, customrate_check_timer, sd->id, 0);
}

static void customrate_apply_effect(map_session_data* sd, int32 effect) {
	if (!sd)
		return;
	if (effect < 0 || effect >= EFST_MAX)
		effect = 0;
	if (sd->customrate.effect > 0)
		clif_status_change(sd, sd->customrate.effect, 0, 0, 0, 0, 0);
	sd->customrate.effect = effect;
	if (effect > 0)
		clif_status_change(sd, effect, 1, INFINITE_TICK, 0, 0, 0);
}

static void customrate_clear_bonus(map_session_data* sd) {
	if (sd)
		pc_bonus_script_clear(sd, CUSTOMRATE_BONUS_FLAG);
}

static void customrate_apply_bonus(map_session_data* sd, const s_customrate_db& rate) {
	if (!sd)
		return;
	customrate_clear_bonus(sd);
	if (rate.bonus_script.empty())
		return;
	const uint16 flags = static_cast<uint16>(CUSTOMRATE_BONUS_FLAG | BSF_REM_ON_LOGOUT);
	struct s_bonus_script_entry* entry = pc_bonus_script_add(sd, rate.bonus_script.c_str(), CUSTOMRATE_BONUS_DURATION, EFST_BLANK, flags, 1);
	if (!entry)
		return;
	linkdb_insert(&sd->bonus_script.head, (void*)((intptr_t)entry), entry);
	status_calc_pc(sd, SCO_NONE);
}

static void customrate_apply_normal(map_session_data* sd) {
	if (!sd)
		return;
	customrate_clear_bonus(sd);
	customrate_apply_effect(sd, 0);
	sd->customrate.base_rate = 1;
	sd->customrate.job_rate = 1;
	sd->customrate.drop_rate = 1;
}

static bool customrate_run_script(map_session_data* sd, struct script_code* code) {
	if (!sd || !code || sd->customrate.running_script)
		return false;
	sd->customrate.running_script = true;
	run_script(code, 0, sd->id, 0);
	sd->customrate.running_script = false;
	return true;
}

static void customrate_cancel_script_timer(map_session_data* sd) {
	if (!sd)
		return;
	if (sd->customrate.script_timer != INVALID_TIMER) {
		delete_timer(sd->customrate.script_timer, customrate_script_timer);
		sd->customrate.script_timer = INVALID_TIMER;
	}
}

static void customrate_schedule_script(map_session_data* sd, uint32 rate_id, bool script_end) {
	if (!sd || !rate_id)
		return;
	customrate_cancel_script_timer(sd);
	sd->customrate.script_rate_id = static_cast<int32>(rate_id);
	sd->customrate.script_end_pending = script_end;
	sd->customrate.script_timer = add_timer(gettick() + 100, customrate_script_timer, sd->id, 0);
}

TIMER_FUNC(customrate_script_timer) {
	map_session_data* sd = map_id2sd(id);
	if (!sd)
		return 1;
	if (sd->customrate.script_timer != tid)
		return 0;
	sd->customrate.script_timer = INVALID_TIMER;
	const uint32 rate_id = static_cast<uint32>(sd->customrate.script_rate_id);
	const bool script_end = sd->customrate.script_end_pending;
	sd->customrate.script_rate_id = 0;
	sd->customrate.script_end_pending = false;
	const std::shared_ptr<s_customrate_db> rate = customrate_search(rate_id);
	if (!rate)
		return 0;
	if (!script_end && (sd->customrate.state != CUSTOM_RATE_ACTIVE || sd->customrate.id != static_cast<int32>(rate_id)))
		return 0;
	if (script_end)
		customrate_run_script(sd, rate->script_end);
	else {
		customrate_apply_bonus(sd, *rate);
		customrate_run_script(sd, rate->script);
	}
	return 0;
}

static bool customrate_give_first_rewards(map_session_data* sd, const s_customrate_db& rate) {
	if (!sd || rate.first_rewards.empty())
		return true;

	char reward_var[64];
	snprintf(reward_var, sizeof(reward_var), "%s%u", CUSTOMRATE_REWARD_PREFIX, rate.id);
	if (pc_readglobalreg(sd, add_str(reward_var)) != 0)
		return true;

	for (const s_customrate_reward& reward : rate.first_rewards) {
		if (pc_checkadditem(sd, reward.item_id, reward.amount) == CHKADDITEM_OVERAMOUNT) {
			customrate_message(sd, "Temporada: libere espaco no inventario para receber a recompensa.");
			return false;
		}
	}

	for (const s_customrate_reward& reward : rate.first_rewards) {
		struct item item = {};
		item.nameid = reward.item_id;
		item.amount = reward.amount;
		item.identify = 1;
		if (pc_additem(sd, &item, reward.amount, LOG_TYPE_NONE) != ADDITEM_SUCCESS) {
			customrate_message(sd, "Temporada: nao foi possivel entregar toda a recompensa.");
			return false;
		}
	}

	pc_setglobalreg(sd, add_str(reward_var), 1);
	customrate_message(sd, "Temporada: recompensa inicial entregue.");
	return true;
}

static void customrate_start_from_db(map_session_data* sd, const s_customrate_db& rate) {
	nullpo_retv(sd);
	customrate_cancel_first_login_timer(sd);
	customrate_cancel_timer(sd);
	sd->customrate.id = rate.id;
	sd->customrate.state = CUSTOM_RATE_ACTIVE;
	sd->customrate.base_rate = std::max(1, rate.base_rate);
	sd->customrate.job_rate = std::max(1, rate.job_rate);
	sd->customrate.drop_rate = std::max(1, rate.drop_rate);
	sd->customrate.expires_at = rate.duration > 0 ? time(nullptr) + rate.duration : 0;
	customrate_apply_effect(sd, rate.effect);
	customrate_save_state(sd);
	customrate_schedule_timer(sd);
	customrate_give_first_rewards(sd, rate);
	customrate_schedule_script(sd, rate.id, false);
}

std::string customrate_get_time_string(const map_session_data* sd) {
	if (!sd || sd->customrate.state != CUSTOM_RATE_ACTIVE)
		return "Nenhuma temporada ativa.";
	std::shared_ptr<s_customrate_db> rate = customrate_search(sd->customrate.id);
	if (!rate)
		return "Temporada invalida.";
	if (sd->customrate.expires_at == 0)
		return rate->name + ": tempo ilimitado.";
	const time_t remaining = sd->customrate.expires_at - time(nullptr);
	if (remaining <= 0)
		return rate->name + ": expirada.";
	const int64 seconds = static_cast<int64>(remaining);
	const int64 days = seconds / 86400;
	const int64 hours = (seconds % 86400) / 3600;
	const int64 minutes = (seconds % 3600) / 60;
	char output[CHAT_SIZE_MAX];
	snprintf(output, sizeof(output), "%s: %lldd %lldh %lldmin restantes.", rate->name.c_str(),
		static_cast<long long>(days), static_cast<long long>(hours), static_cast<long long>(minutes));
	return output;
}

std::string customrate_get_info(uint32 id, int32 type) {
	std::shared_ptr<s_customrate_db> rate = customrate_search(id);
	if (!rate)
		return {};
	if (type == CUSTOM_RATE_INFO_NAME)
		return rate->name;
	if (type == CUSTOM_RATE_INFO_TIME)
		return std::to_string(static_cast<long long>(rate->duration));
	if (type == CUSTOM_RATE_INFO_BASE)
		return std::to_string(rate->base_rate);
	if (type == CUSTOM_RATE_INFO_JOB)
		return std::to_string(rate->job_rate);
	if (type == CUSTOM_RATE_INFO_DROP)
		return std::to_string(rate->drop_rate);
	return {};
}

bool customrate_select(map_session_data* sd, uint32 id) {
	nullpo_retr(false, sd);
	if (!battle_config.customrate_enable)
		return false;
	std::shared_ptr<s_customrate_db> rate = customrate_search(id);
	if (!rate)
		return false;

	if (sd->customrate.state == CUSTOM_RATE_ACTIVE && sd->customrate.id == static_cast<int32>(id))
		return true;

	// Switching rates does not execute the old end script. Expiry does.
	customrate_reset(sd, false);
	customrate_start_from_db(sd, *rate);
	pc_setglobalreg(sd, add_str(CUSTOMRATE_FIRST_LOGIN_VAR), 1);
	char message[CHAT_SIZE_MAX];
	snprintf(message, sizeof(message), "Temporada ativada: %s | EXP Base %dx | EXP Job %dx | Drop %dx.",
		rate->name.c_str(), rate->base_rate, rate->job_rate, rate->drop_rate);
	customrate_message(sd, message);
	return true;
}

bool customrate_reset(map_session_data* sd, bool run_end_script) {
	nullpo_retr(false, sd);
	std::shared_ptr<s_customrate_db> old_rate = sd->customrate.id > 0 ? customrate_search(sd->customrate.id) : nullptr;
	struct script_code* end_script = old_rate ? old_rate->script_end : nullptr;
	customrate_cancel_first_login_timer(sd);
	customrate_cancel_script_timer(sd);
	customrate_cancel_timer(sd);
	customrate_apply_normal(sd);
	sd->customrate.id = 0;
	sd->customrate.state = CUSTOM_RATE_INACTIVE;
	sd->customrate.expires_at = 0;
	customrate_save_state(sd);
	if (run_end_script && end_script && old_rate)
		customrate_schedule_script(sd, old_rate->id, true);
	return true;
}

bool customrate_extend(map_session_data* sd, int32 days) {
	nullpo_retr(false, sd);
	if (days < 1 || days > 30 || sd->customrate.state != CUSTOM_RATE_ACTIVE || sd->customrate.expires_at <= 0)
		return false;
	const time_t now = time(nullptr);
	const time_t start = std::max(now, sd->customrate.expires_at);
	sd->customrate.expires_at = start + static_cast<time_t>(days) * 86400;
	customrate_save_state(sd);
	customrate_schedule_timer(sd);
	return true;
}

void customrate_char_load(map_session_data* sd) {
	nullpo_retv(sd);
	sd->customrate = {};
	sd->customrate.timer = INVALID_TIMER;
	sd->customrate.id = static_cast<int32>(pc_readglobalreg(sd, add_str(CUSTOMRATE_ID_VAR)));
	sd->customrate.state = static_cast<int32>(pc_readglobalreg(sd, add_str(CUSTOMRATE_STATE_VAR)));
	sd->customrate.expires_at = static_cast<time_t>(pc_readglobalreg(sd, add_str(CUSTOMRATE_TIME_VAR)));
	sd->customrate.first_login_timer = INVALID_TIMER;
	if (sd->customrate.state != CUSTOM_RATE_ACTIVE || sd->customrate.id <= 0) {
		customrate_apply_normal(sd);
		sd->customrate.id = 0;
		sd->customrate.state = CUSTOM_RATE_INACTIVE;
		customrate_message(sd, "Bem-vindo! Fale com o Gerente de Temporada em Prontera para escolher sua rate pessoal.");
		return;
	}
	std::shared_ptr<s_customrate_db> rate = customrate_search(sd->customrate.id);
	if (!rate || (sd->customrate.expires_at > 0 && sd->customrate.expires_at <= time(nullptr))) {
		customrate_reset(sd, true);
		return;
	}
	sd->customrate.base_rate = rate->base_rate;
	sd->customrate.job_rate = rate->job_rate;
	sd->customrate.drop_rate = rate->drop_rate;
	customrate_apply_effect(sd, rate->effect);
	customrate_schedule_timer(sd);
	customrate_schedule_script(sd, rate->id, false);
}

void customrate_quit(map_session_data* sd) {
	if (sd) {
		customrate_cancel_first_login_timer(sd);
		customrate_cancel_script_timer(sd);
		customrate_cancel_timer(sd);
	}
}

void customrate_on_levelup(map_session_data* sd) {
	if (!sd || sd->customrate.state != CUSTOM_RATE_ACTIVE)
		return;
	std::shared_ptr<s_customrate_db> rate = customrate_search(sd->customrate.id);
	if (rate)
		customrate_schedule_script(sd, rate->id, false);
}

int32 customrate_base_rate(const map_session_data* sd) {
	return sd && sd->customrate.state == CUSTOM_RATE_ACTIVE ? std::max(1, sd->customrate.base_rate) : 1;
}

int32 customrate_job_rate(const map_session_data* sd) {
	return sd && sd->customrate.state == CUSTOM_RATE_ACTIVE ? std::max(1, sd->customrate.job_rate) : 1;
}

int32 customrate_drop_rate(const map_session_data* sd) {
	return sd && sd->customrate.state == CUSTOM_RATE_ACTIVE ? std::max(1, sd->customrate.drop_rate) : 1;
}

TIMER_FUNC(customrate_check_timer) {
	map_session_data* sd = map_id2sd(id);
	if (!sd)
		return 1;
	if (sd->customrate.timer != tid)
		return 0;
	sd->customrate.timer = INVALID_TIMER;
	if (sd->customrate.state != CUSTOM_RATE_ACTIVE || sd->customrate.expires_at <= 0)
		return 0;
	if (sd->customrate.expires_at <= time(nullptr)) {
		customrate_message(sd, "Sua temporada terminou. A rate normal foi restaurada.");
		customrate_reset(sd, true);
		return 0;
	}
	customrate_schedule_timer(sd);
	return 0;
}

void customrate_reload() {
	do_final_customrate();
	customrate_db.load();
}

void do_init_customrate() {
	add_timer_func_list(customrate_check_timer, "customrate_check_timer");
	add_timer_func_list(customrate_script_timer, "customrate_script_timer");
	customrate_db.load();
	ShowStatus("Custom Season/Rate Engine loaded.\n");
}

void do_final_customrate() {
	for (const auto& entry : customrate_db) {
		const std::shared_ptr<s_customrate_db>& rate = entry.second;
		if (rate->script)
			script_free_code(rate->script);
		if (rate->script_end)
			script_free_code(rate->script_end);
	}
	customrate_db.clear();
}
