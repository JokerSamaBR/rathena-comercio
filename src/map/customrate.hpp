// Custom Season / Custom Rate Engine
// Ported and revised for this rAthena tree.

#ifndef CUSTOMRATE_HPP
#define CUSTOMRATE_HPP

#include <ctime>
#include <memory>
#include <string>
#include <vector>

#include <common/cbasetypes.hpp>
#include <common/database.hpp>
#include <common/db.hpp>
#include <common/mmo.hpp>
#include <common/timer.hpp>

class map_session_data;
struct script_code;

enum e_customrate_state : uint8 {
	CUSTOM_RATE_INACTIVE = 0,
	CUSTOM_RATE_ACTIVE = 1,
	CUSTOM_RATE_EXPIRED = 2,
};

enum e_customrate_info : uint8 {
	CUSTOM_RATE_INFO_NAME = 0,
	CUSTOM_RATE_INFO_TIME = 1,
	CUSTOM_RATE_INFO_BASE = 2,
	CUSTOM_RATE_INFO_JOB = 3,
	CUSTOM_RATE_INFO_DROP = 4,
};

struct s_customrate_data {
	int32 id = 0;
	int32 state = CUSTOM_RATE_INACTIVE;
	int32 base_rate = 1;
	int32 job_rate = 1;
	int32 drop_rate = 1;
	int32 effect = 0;
	time_t expires_at = 0;
	int32 timer = INVALID_TIMER;
	int32 first_login_timer = INVALID_TIMER;
	int32 script_timer = INVALID_TIMER;
	int32 script_rate_id = 0;
	bool script_end_pending = false;
	bool running_script = false;
};

struct s_customrate_reward {
	t_itemid item_id = 0;
	int32 amount = 1;
};

struct s_customrate_db {
	uint32 id = 0;
	std::string name;
	int32 base_rate = 1;
	int32 job_rate = 1;
	int32 drop_rate = 1;
	int32 effect = 0;
	int32 title_id = 0;
	time_t duration = 0;
	std::vector<s_customrate_reward> first_rewards;
	std::string bonus_script;
	struct script_code* script = nullptr;
	struct script_code* script_end = nullptr;
};

class CustomRateDatabase : public TypesafeYamlDatabase<uint32, s_customrate_db> {
public:
	CustomRateDatabase() : TypesafeYamlDatabase("CUSTOM_RATE_DB", 1) {}
	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef& node) override;
};

extern CustomRateDatabase customrate_db;

std::shared_ptr<s_customrate_db> customrate_search(uint32 id);
const s_customrate_data* customrate_get_state(const map_session_data* sd);
int32 customrate_get_id(const map_session_data* sd);
int32 customrate_get_state_value(const map_session_data* sd);
time_t customrate_get_time(const map_session_data* sd);
std::string customrate_get_time_string(const map_session_data* sd);
std::string customrate_get_info(uint32 id, int32 type);

bool customrate_select(map_session_data* sd, uint32 id);
bool customrate_reset(map_session_data* sd, bool run_end_script = true);
bool customrate_extend(map_session_data* sd, int32 days);
void customrate_char_load(map_session_data* sd);
void customrate_quit(map_session_data* sd);
void customrate_on_levelup(map_session_data* sd);
void customrate_reload();

/// Applies the selected individual rate to the actual EXP/drop path.
int32 customrate_base_rate(const map_session_data* sd);
int32 customrate_job_rate(const map_session_data* sd);
int32 customrate_drop_rate(const map_session_data* sd);

TIMER_FUNC(customrate_check_timer);
void do_init_customrate();
void do_final_customrate();

#endif
