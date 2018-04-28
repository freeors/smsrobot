#ifndef GAME_CONFIG_H_INCLUDED
#define GAME_CONFIG_H_INCLUDED

#include "preferences.hpp"
#include "sdl_utils.hpp"
#include "gui/widgets/widget.hpp"
#include <google/protobuf/message.h>

enum {CONTACTS_PB, TEMPLATES_PB};
enum {sim_none, sim_sim0, sim_sim1, sim_alternate, sim_max = sim_alternate};

struct tsim_info {
	tsim_info()
		: level22(false)
	{}

	tsim_info(int slot, const std::string& carrier, const std::string& number)
		: level22(true)
		, slot(slot)
		, carrier(carrier)
		, number(number)
	{}

	std::string description() const;

	bool level22;
	int slot; // base 0
	std::string carrier;
	std::string number;
};

struct tsend_info {
	tsend_info()
		: running(false)
		, unrender_record_at(nposm)
		, sim_strategy(sim_none)
		, next_send_at(0)
	{}

	void set(int _sim_strategy, int _interval, uint32_t _last_send_ticks)
	{
		VALIDATE(_interval > 0 && _interval <= 24 * 3600, null_str);

		sim_strategy = _sim_strategy;
		interval = _interval;

		const int first_send_delay = 2; // 2 second.
		last_send_ticks = _last_send_ticks - interval * 1000 + first_send_delay * 1000;
	}

	bool valid() const { return sim_strategy != sim_none; }
	bool can_edit_contacts_or_templates() const { return !running && !next_send_at; }

	bool running;
	int unrender_record_at;

	int sim_strategy; 
	int interval; // second

	int last_send_ticks;
	int last_use_slot;
	int next_send_at;
};

struct tsend_record {
	tsend_record(const time_t t, int slot, const std::string& receiver, const std::string& message)
		: t(t)
		, slot(slot)
		, receiver(receiver)
		, message(message)
	{}

	time_t t;
	int slot;
	std::string receiver;
	std::string message;
};

std::string sim_strategy_short_description(const int sim);

struct tparameters {
	void push_back(const std::string& id, const std::string& name);
	bool is_existed(const std::string& id) const;
	const std::string& name(const std::string& id) const;

	std::vector<std::pair<std::string, std::string> > map;
};

namespace pb {
	class tbh_field;
	class tperson;
	class tcontacts;
	class ttemplate;
	class ttemplates;
}

std::string generate_highlight_template_field(const std::string& field_id, const std::string& name);
const pb::tbh_field* find_bh_field_from_contacts(const pb::tcontacts& contacts, const std::string& field_id);
std::set<int> template_extract_parameters(const std::string& message);
std::string template_generate_preview(const tparameters& parameters, const pb::tcontacts& contacts, const pb::ttemplate& template2);
std::string template_generate_message2(const tparameters& parameters, const pb::ttemplate& template2, const pb::tperson& person);
void generate_send_queue(tsend_info& send_info, std::vector<tsend_record>& send_records, const pb::tcontacts& contacts, const pb::ttemplates& templates, const tparameters& parameters, const time_t first_t);
void load_pb(int type, ::google::protobuf::Message& lite);

//basic game configuration information is here.
namespace game_config {
	extern int record_threshold;
	extern const std::string markup_cookie_key;
}

namespace preferences {
	int sim_strategy();
	void set_sim_strategy(int sim);

	int interval();
	void set_interval(int interval);

	int next_send_at();
	void set_next_send_at(int next_send_at);

	std::string bbs_server_furl();
	void set_bbs_server_furl(const std::string& furl);
}

#endif
