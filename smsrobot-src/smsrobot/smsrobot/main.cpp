/* Require Rose v1.0.10 or above. $ */

#define GETTEXT_DOMAIN "smsrobot-lib"

#include "base_instance.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/chat.hpp"
#include "gui/dialogs/home.hpp"
#include "gui/dialogs/edit_group.hpp"
#include "gui/dialogs/edit_person.hpp"
#include "gui/dialogs/edit_template.hpp"
#include "gui/dialogs/edit_field.hpp"
#include "gui/dialogs/combo_box2.hpp"
#include "gui/dialogs/login.hpp"
#include "gui/dialogs/register.hpp"
#include "gui/dialogs/notification.hpp"
#include "gui/dialogs/ocr.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/timer.hpp"
#include "game_end_exceptions.hpp"
#include "wml_exception.hpp"
#include "gettext.hpp"
#include "loadscreen.hpp"
#include "formula_string_utils.hpp"
#include "help.hpp"
#include "sms_smart_android.h"
#include "language.hpp"
#include "version.hpp"
#include "network.hpp"
#include "tflite.hpp"
#include "hotkeys.hpp"

#include "contacts.pb.h"
#include "templates.pb.h"

#include <boost/bind.hpp>
#include <boost/regex.hpp>

const pb::tbh_field* find_bh_field_from_contacts(const pb::tcontacts& contacts, const std::string& field_id)
{
	int bh_fields = contacts.bh_fields_size();
	for (int at = 0; at < bh_fields; at ++) {
		const pb::tbh_field& bh_field = contacts.bh_fields(at);
		if (bh_field.id() == field_id) {
			return &bh_field;
		}
	}
	return nullptr;
}

std::set<int> template_extract_parameters(const std::string& message)
{
	std::set<int> result;

	// %[0-9]{2}
	boost::regex re("%([0-9]{2})");
	boost::smatch m;
	std::string::const_iterator start = message.begin();
	std::string::const_iterator end = message.end();
	while (boost::regex_search(start, end, m, re, boost::regex_constants::match_not_dot_null)) {
		const std::string m1 = m[1];
		start = m[0].second;

		int index = atoi(m1.c_str());
		result.insert(index);
	}
	
	return result;
}

std::string template_generate_message(const std::string& template2, const std::map<int, std::string>& parameters)
{
	std::stringstream result;
	std::map<int, std::string>::const_iterator find;
	int search_start_at = 0;
	// %[0-9]{2}
	boost::regex re("%([0-9]{2})");
	boost::smatch m;
	std::string::const_iterator start = template2.begin();
	std::string::const_iterator end = template2.end();
	while (boost::regex_search(start, end, m, re, boost::regex_constants::match_not_dot_null)) {
		const std::string m1 = m[1];
		start = m[0].second;

		const int new_search_start_at = std::distance(template2.begin(), start);
		result << template2.substr(search_start_at,  new_search_start_at - search_start_at - 3);
		search_start_at = new_search_start_at;

		int index = atoi(m1.c_str());
		find = parameters.find(index);
		if (find != parameters.end()) {
			result << find->second;
		} else {
			// keep source.
			result << "%" << m1;
		}
	}
	
	result << template2.substr(search_start_at);

	return result.str();
}

std::string generate_highlight_template_field(const std::string& field_id, const std::string& name)
{
	std::map<std::string, std::string> extra;
	extra.insert(std::make_pair(game_config::markup_cookie_key, field_id));
	return ht::generate_format(name, 0xffff0000, 0, false, false, extra);
}

std::string template_generate_preview(const tparameters& parameters, const pb::tcontacts& contacts, const pb::ttemplate& template2)
{
	if (template2.message().empty()) {
		return null_str;
	}
	std::map<int, std::string> parameters2;

	std::set<int> indexs = template_extract_parameters(template2.message());
	const int params = template2.params_size();
	for (std::set<int>::const_iterator it = indexs.begin(); it != indexs.end(); ++ it) {
		const int index = *it;
		for (int at = 0; at < params; at ++) {
			if (template2.params(at).index() == index) {
				const std::string& field_id = template2.params(at).field_id();
				if (field_id.empty()) {
					// no set.
					break;
				}
				std::string name;
				if (parameters.is_existed(field_id)) {
					name = parameters.name(field_id);
				} else {
					const pb::tbh_field* bh_field = find_bh_field_from_contacts(contacts, field_id);
					if (bh_field) {
						name = bh_field->name();
					}
				}
				parameters2.insert(std::make_pair(index, generate_highlight_template_field(field_id, name)));
				break;
			}
		}
	}

	return template_generate_message(template2.message(), parameters2);
}

std::string template_generate_message2(const tparameters& parameters, const pb::ttemplate& template2, const pb::tperson& person)
{
	if (template2.message().empty()) {
		return null_str;
	}
	std::map<int, std::string> parameters2;

	std::set<int> indexs = template_extract_parameters(template2.message());
	const int params = template2.params_size();
	for (std::set<int>::const_iterator it = indexs.begin(); it != indexs.end(); ++ it) {
		const int index = *it;
		for (int at = 0; at < params; at ++) {
			if (template2.params(at).index() == index) {
				const std::string& field_id = template2.params(at).field_id();
				if (field_id.empty()) {
					// no set.
					break;
				}
				std::string value;
				if (field_id == "name") {
					value = person.name();
				} else if (field_id == "mobile_number") {
					value = person.mobiles(0).number();
				} else if (field_id == "money") {
					value = person.money();
				} else {
					const int bh_fields = person.bh_fields_size();
					for (int at2 = 0; at2 < bh_fields; at2 ++) {
						const pb::tperson_bh_field& bh_field = person.bh_fields(at2);
						if (bh_field.field_id() == field_id) {
							value = bh_field.value();
							break;
						}
					}
				}
				parameters2.insert(std::make_pair(index, value));
				break;
			}
		}
	}

	return template_generate_message(template2.message(), parameters2);
}

void generate_send_queue(tsend_info& send_info, std::vector<tsend_record>& send_records, const pb::tcontacts& contacts, const pb::ttemplates& templates, const tparameters& parameters, const time_t first_t)
{
	VALIDATE(send_info.can_edit_contacts_or_templates(), null_str);

	send_records.clear();
	if (!send_info.valid()) {
		return;
	}
	if (!contacts.groups_size() || !templates.templates_size()) {
		return;
	}

	const pb::tgroup& current_group = contacts.groups(0);
	const pb::ttemplate& current_template = templates.templates(0);

	if (!current_group.persons_size() || current_template.message().empty()) {
		return;
	}

	const int persons = current_group.persons_size();
	if (!persons) {
		return;
	}

	int last_use_slot = 0;
	time_t t = first_t;
	for (int person_at = 0; person_at < persons; person_at ++) {
		const pb::tperson& person = current_group.persons(person_at);
		for (int number_at = 0; number_at < person.mobiles_size(); number_at ++) {
			const pb::tperson_mobile& mobile = person.mobiles(number_at);
			if (send_info.sim_strategy == sim_sim0) {
				last_use_slot = 0;
			} else if (send_info.sim_strategy == sim_sim1) {
				last_use_slot = 1;
			} else {
				VALIDATE(send_info.sim_strategy == sim_alternate, null_str);
				if (send_info.last_use_slot != 0) {
					last_use_slot = 0;
				} else {
					last_use_slot = 1;
				}
			}

			std::string message = template_generate_message2(parameters, current_template, person);
			send_records.push_back(tsend_record(t, last_use_slot, mobile.number(), message));
			t += send_info.interval;
		}
	}

	// std::reverse(send_records.begin(), send_records.end());
}

void load_pb(int type, ::google::protobuf::Message& lite)
{
	std::string file_name;
	if (type == CONTACTS_PB) {
		file_name = "contacts";
	} else {
		VALIDATE(type == TEMPLATES_PB, null_str);
		file_name = "templates";
	}
	tfile file(game_config::preferences_dir + "/" + file_name + ".pb", GENERIC_READ, OPEN_EXISTING);

	int fsize = file.read_2_data();
	if (fsize) {
		lite.ParseFromArray(file.data, fsize);
	}
}

class game_instance: public base_instance
{
public:
	class thome_lock {
	public:
		thome_lock(game_instance& instance, gui2::thome& home)
			: instance_(instance)
		{
			VALIDATE(!instance_.home_, null_str);
			instance_.home_ = &home;
		}
		~thome_lock()
		{
			instance_.home_ = nullptr;
		}

	private:
		game_instance& instance_;			
	};
	game_instance(rtc::PhysicalSocketServer& ss, int argc, char** argv);

	tsend_info& send_info() { return send_info_; }
	std::vector<tsend_record>& send_records() { return send_records_; }
	tsms_smart& sms_smart() { return sms_smart_; }
	const std::vector<tsim_info>& sim_infos() const { return sim_infos_; }
	const tparameters& parameters() const { return parameters_; }

	pb::tcontacts& contacts() { return contacts_; }
	pb::ttemplates& templates() { return templates_; }

	void insert_group(const pb::tgroup& group);
	void insert_template(const pb::ttemplate& template2);
	void insert_bh_field(const pb::tbh_field& bh_field);

	void run_task(const bool startup);
	void handle_login();
	std::map<std::string, tocr_result> handle_ocr(const std::vector<std::string>& fields);

	void load_pb() override;
	void write_pb(int type) override;

private:
	void app_load_settings_config(const config& cfg) override;
	void app_didenterbackground() override;
	void app_didenterforeground() override;

	bool did_background(uint32_t now, bool screen_on);
	void send_sms();
	void send_timer_handler();

private:
	gui2::thome* home_;
	tsend_info send_info_;
	std::vector<tsend_record> send_records_;
	gui2::ttimer send_timer_;
	std::vector<tsim_info> sim_infos_; 
	tsms_smart sms_smart_;
	tparameters parameters_;

	bool require_refresh_history_;
	pb::tcontacts contacts_;
	pb::ttemplates templates_;
};

game_instance::game_instance(rtc::PhysicalSocketServer& ss, int argc, char** argv)
	: base_instance(ss, argc, argv)
	, home_(nullptr)
	, require_refresh_history_(false)
{
}

class treset_next_send_at_lock
{
public:
	treset_next_send_at_lock(tsend_info& send_info)
		: send_info_(send_info)
		, original_next_send_at_(send_info.next_send_at)
	{
		send_info_.next_send_at = 0;
	}
	~treset_next_send_at_lock()
	{
		send_info_.next_send_at = original_next_send_at_;
	}

private:
	tsend_info& send_info_;
	int original_next_send_at_;
};

void game_instance::load_pb()
{
	parameters_.push_back("name", _("person^Name"));
	parameters_.push_back("mobile_number", _("mobile^Number"));
	parameters_.push_back("money", _("money^Sum"));

	// pb
	sms_smart_.get_siminfo(sim_infos_);
	VALIDATE(sim_infos_.size() <= 2, null_str);

	::load_pb(CONTACTS_PB, contacts_);
	::load_pb(TEMPLATES_PB, templates_);

	const std::string furl = preferences::bbs_server_furl();
	if (!furl.empty()) {
		game_config::bbs_server.split_furl(furl, nullptr);
	}

	// send infomation
	send_info_.set(preferences::sim_strategy(), preferences::interval(), SDL_GetTicks());
	send_info_.next_send_at = preferences::next_send_at();
	if (send_info_.valid()) {
		{
			treset_next_send_at_lock lock(send_info_);
			generate_send_queue(send_info_, send_records_, contacts_, templates_, parameters_, time(nullptr));
		}
		if (send_info_.next_send_at > (int)send_records_.size()) {
			// maybe equal
			send_info_.next_send_at = 0;
		}

	} else {
		send_info_.next_send_at = 0;
	}
	preferences::set_next_send_at(send_info_.next_send_at);

	// start timer
	send_timer_.reset(gui2::add_timer(1000, boost::bind(&game_instance::send_timer_handler, this)));
}

void game_instance::write_pb(int type)
{
	std::string file_name;
	if (type == CONTACTS_PB) {
		contacts_.set_timestamp(time(nullptr));
		file_name = "contacts";
	} else if (type == TEMPLATES_PB) {
		templates_.set_timestamp(time(nullptr));
		file_name = "templates";
	} else {
		return;
	}
	tfile file(game_config::preferences_dir + "/" + file_name + ".pb", GENERIC_WRITE, CREATE_ALWAYS);

	::google::protobuf::Message* lite = nullptr;
	if (type == CONTACTS_PB) {
		lite = &dynamic_cast<game_instance*>(instance)->contacts();
	} else if (type == TEMPLATES_PB) {
		lite = &dynamic_cast<game_instance*>(instance)->templates();
	}
	int size = lite->ByteSize();
	file.resize_data(size);
	lite->SerializeToArray(file.data, size);

	posix_fwrite(file.fp, file.data, size);
	file.close();

	if (home_ && foreground_) {
		home_->refresh_notification_widget(nposm);
	}
}

void game_instance::app_load_settings_config(const config& cfg)
{
	const config& bbs_server_cfg = cfg.child("bbs_server");
	game_config::bbs_server.from_cfg(bbs_server_cfg? bbs_server_cfg: null_cfg);
}

bool game_instance::did_background(uint32_t now, bool screen_on)
{
	if (!send_info_.running) {
		// send task maybe completment when background.
		return true;
	}

	VALIDATE(!send_timer_.valid(), null_str);
/*
#ifdef ANDROID
	if (instance->minimized() && !screen_on && !Mix_PlayingMusic()) {
		sound::play_music_repeatedly("babycry.ogg");

	} else if (screen_on && Mix_PlayingMusic()) {
		// I think, babycry.ogg is the only music.
		sound::stop_music();
	}
#endif
*/
	if ((int)now - send_info_.last_send_ticks >= send_info_.interval * 1000) {
		send_sms();
	}
	return false;
}

void game_instance::app_didenterbackground()
{
	VALIDATE(send_timer_.valid(), null_str);
	send_timer_.reset();
	if (send_info_.running) {
		instance->background_connect(boost::bind(&game_instance::did_background, this, _1, _2));
	}
}

void game_instance::app_didenterforeground()
{
	VALIDATE(!send_timer_.valid(), null_str);
	send_timer_.reset(gui2::add_timer(1000, boost::bind(&game_instance::send_timer_handler, this)));
	require_refresh_history_ = true;
}

void game_instance::insert_group(const pb::tgroup& group)
{
	pb::tgroup* new_group = contacts_.add_groups();
	*new_group = group;
}

void game_instance::insert_template(const pb::ttemplate& template2)
{
	pb::ttemplate* new_template = templates_.add_templates();
	*new_template = template2;
}

void game_instance::insert_bh_field(const pb::tbh_field& bh_field)
{
	pb::tbh_field* new_bh_field = contacts_.add_bh_fields();
	*new_bh_field = bh_field;
}

void game_instance::run_task(const bool startup)
{
	if (startup) {
		VALIDATE(send_timer_.valid() && foreground_, null_str);
		VALIDATE(send_info_.valid() && !send_info_.running, null_str);
		VALIDATE(!send_records_.empty(), null_str);

		send_info_.running = true;
	} else {
		VALIDATE(send_info_.running, null_str);

		send_info_.running = false;
	}
}

void game_instance::send_sms()
{
	// slot
	VALIDATE(send_info_.next_send_at < (int)send_records_.size(), null_str);
	tsend_record& record = send_records_[send_info_.next_send_at];

	sms_smart_.send(record.slot, record.receiver, record.message);

	SDL_Log("send_sms(), slot(%i), receiver(%s), foreground: %s\n", record.slot, record.receiver.c_str(), instance->foreground()? "true": "false");
	if (!instance->foreground()) {
		sound::play_sound("sms.wav");
	}

	send_info_.last_send_ticks = (int)SDL_GetTicks();
	record.t = time(nullptr);

	if (home_ && foreground_) {
		home_->did_sms_sent(send_info_.next_send_at);
	}

	send_info_.next_send_at ++;
	// if (foreground_) {
		preferences::set_next_send_at(send_info_.next_send_at);
	// }
	if (send_info_.next_send_at == send_records_.size()) {
		SDL_Log("send_sms, send_info_.next_send_at(%i) == send_records_.size()(%i), will call run_task(false).\n",
			send_info_.next_send_at, send_records_.size());
		run_task(false);
		if (home_ && foreground_) {
			home_->did_refresh_task_buttons();
		}
	}
}

void game_instance::send_timer_handler()
{
	if (send_info_.running) {
		uint32_t now = SDL_GetTicks();
		if ((int)now - send_info_.last_send_ticks >= send_info_.interval * 1000) {
			send_sms();
		}
	}

	if (home_ && require_refresh_history_) {
		home_->did_refresh_task_buttons();
		home_->did_refresh_queue_list(false);
		require_refresh_history_ = false;
	}
}

void game_instance::handle_login()
{
	gui2::tlogin::tresult res;
	while (true) {
		{
			gui2::tlogin dlg;
			dlg.show();
			res = static_cast<gui2::tlogin::tresult>(dlg.get_retval());
		}
		if (res == gui2::tlogin::REGISTER) {
			gui2::tregister dlg(false);
			dlg.show();
			if (dlg.get_retval() == gui2::twindow::OK) {
				break;
			}


		} else {
			break;
		}
	}
}

std::map<std::string, tocr_result> game_instance::handle_ocr(const std::vector<std::string>& fields)
{
	const std::string pb_short_path = "combined_model.pb";

	surface surf = image::get_image("misc/template_ocr.png");
	// surface surf = image::get_image(game_config::preferences_dir + "/3.png");
	// surface surf;
	// std::map<std::string, tocr_result> ret = tflite::ocr(app_cfg(), video_, surf, fields, null_str, pb_short_path, false);
	std::map<std::string, tocr_result> ret;
	if (ret.empty()) {
		return ret;
	}
	std::stringstream msg_ss;
	for (std::map<std::string, tocr_result>::const_iterator it = ret.begin(); it != ret.end(); ++ it) {
		if (!msg_ss.str().empty()) {
			msg_ss << "\n";
		}
		msg_ss << it->first << ": " << it->second.chars << ", used ticks: " << it->second.used_ms;
	}
	gui2::show_message(null_str, msg_ss.str());

	return ret;
}

static bool is_person_equal(const pb::tperson& a, const pb::tperson& b)
{
	// name
	if (a.name() != b.name()) {
		return false;
	}

	// money
	if (a.money() != b.money()) {
		return false;
	}

	// mobiles
	const int mobiles = a.mobiles_size();
	if (mobiles != b.mobiles_size()) {
		return false;
	}
	for (int at = 0; at < mobiles; at ++) {
		if (a.mobiles(at).number() != b.mobiles(at).number()) {
			return false;
		}
	}

	// bh_fields
	const int bh_fields = a.bh_fields_size();
	if (bh_fields != b.bh_fields_size()) {
		return false;
	}
	for (int at = 0; at < bh_fields; at ++) {
		const pb::tperson_bh_field& a_bh_field = a.bh_fields(at);
		const pb::tperson_bh_field& b_bh_field = b.bh_fields(at);
		if (a_bh_field.field_id() != b_bh_field.field_id()) {
			return false;
		}
		if (a_bh_field.value() != b_bh_field.value()) {
			return false;
		}
	}
	return true;
}

static bool is_group_equal(const pb::tgroup& a, const pb::tgroup& b)
{
	// name
	if (a.name() != b.name()) {
		return false;
	}

	// persons
	const int persons = a.persons_size();
	if (persons != b.persons_size()) {
		return false;
	}
	for (int at = 0; at < persons; at ++) {
		const pb::tperson& a_person = a.persons(at);
		const pb::tperson& b_person = b.persons(at);
		if (!is_person_equal(a_person, b_person)) {
			return false;
		}
	}

	// fields
	const int fields = a.fields_size();
	if (fields != b.fields_size()) {
		return false;
	}
	for (int at = 0; at < fields; at ++) {
		if (a.fields(at) != b.fields(at)) {
			return false;
		}
	}
	return true;
}

static bool is_bh_field_equal(const pb::tbh_field& a, const pb::tbh_field& b)
{
	// id
	if (a.id() != b.id()) {
		return false;
	}

	// name
	if (a.name() != b.name()) {
		return false;
	}

	return true;
}

static bool is_template_equal(const pb::ttemplate& a, const pb::ttemplate& b)
{
	// name
	if (a.name() != b.name()) {
		return false;
	}

	// message
	if (a.message() != b.message()) {
		return false;
	}

	// params
	const int params = a.params_size();
	if (params != b.params_size()) {
		return false;
	}
	for (int at = 0; at < params; at ++) {
		const pb::ttemplate_param& a_param = a.params(at);
		const pb::ttemplate_param& b_param = b.params(at);
		if (a_param.index() != b_param.index()) {
			return false;
		}
		if (a_param.field_id() != b_param.field_id()) {
			return false;
		}
	}

	return true;
}

/**
 * Setups the game environment and enters
 * the titlescreen or game loops.
 */
static int do_gameloop(int argc, char** argv)
{
	rtc::PhysicalSocketServer ss;
	instance_manager<game_instance> manager(ss, argc, argv, "smsrobot", "#rose", false);
	game_instance& game = manager.get();

	try {
		lobby->disable_chat(); // because irc nick protocol, disable chat.

		if (preferences::startup_login()) {
			// login
			std::string username = group.leader().name();
			std::string password = preferences::password();
			int type = preferences::openid_type();

			if (!username.empty()) {
				http_agent agent;
				tuser user = agent.do_login(type, username, password, true);
				if (user.valid()) {
					current_user = user;
				}
			}
		}

		tsend_info& send_info = game.send_info();
		pb::tcontacts& contacts = game.contacts();
		pb::ttemplates& templates = game.templates();
		const tparameters& parameters = game.parameters();
		int start_layer = gui2::thome::SMS_LAYER;
		int context_result;
		for (;;) {
			game.loadscreen_manager().reset();
			const font::floating_label_context label_manager;

			cursor::set(cursor::NORMAL);

			if (!current_user.valid()) {
				game.handle_login();
				start_layer = gui2::thome::SMS_LAYER;
			}

			gui2::thome::tresult res;
			{
				gui2::thome dlg(game.sim_infos(), game.send_info(), game.sms_smart(), game.send_records(), game.contacts(), game.templates(), game.parameters(), boost::bind(&game_instance::run_task, &game, _1), start_layer);

				game_instance::thome_lock lock(game, dlg);
				dlg.show();
				res = static_cast<gui2::thome::tresult>(dlg.get_retval());
				context_result = dlg.context_result();
			}

			if (res == gui2::thome::EDIT_GROUP) {
				const bool edit = context_result != nposm;
				pb::tgroup group;
				if (edit) {
					group = contacts.groups(context_result);
				}
				const int this_group_at = context_result == nposm? contacts.groups_size(): context_result;
				const bool read_only = edit && !send_info.can_edit_contacts_or_templates() && this_group_at == 0;
				bool dirty = false;
				bool contacts_changed = false;

				int context_result2;
				int res2 = gui2::tedit_group::EDIT_PERSON;
				while (res2 == gui2::tedit_group::EDIT_PERSON) {
					{
						gui2::tedit_group dlg(contacts, group, edit, game.parameters(), read_only);
						dlg.show();
						res2 = (gui2::tedit_group::tresult)dlg.get_retval();
						context_result2 = dlg.context_result();
					}
					if (res2 == gui2::tedit_group::EDIT_PERSON) {
						const bool edit2 = context_result2 != nposm;
						pb::tperson person;
						if (edit2) {
							person = group.persons(context_result2);
						}
						std::set<std::string> bh_fields;
						const int proto_fields = group.fields_size();
						for (int at = 0; at < proto_fields; at ++) {
							const std::string& field_id = group.fields(at);
							bh_fields.insert(field_id);
						}

						gui2::tedit_person dlg(contacts, person, edit2, bh_fields, read_only);
						dlg.show();
						start_layer = gui2::thome::GROUP_LAYER;
						if (dlg.get_retval() != gui2::twindow::OK) {
							continue;
						}
						bool dirty2 = false;
						const int this_person_at = context_result2 == nposm? group.persons_size(): context_result2;
						if (!edit2) {
							pb::tperson* new_person = group.add_persons();
							*new_person = person;
							dirty2 = true;
						} else {
							if (!is_person_equal(group.persons(context_result2), person)) {
								*group.mutable_persons(context_result2) = person;
								dirty2 = true;
							}
						}
						if (dirty2) {
							// user maybe edit group long time. save immediate when complete one person.
							if (this_group_at == contacts.groups_size()) {
								VALIDATE(context_result == nposm && !dirty, null_str);
								game.insert_group(group);
							} else {
								VALIDATE(is_person_equal(group.persons(this_person_at), person), null_str);
								// there maybe insert result. use this_group_at replace context_result.
								*contacts.mutable_groups(this_group_at) = group;
							}
							dirty = true;

							game.write_pb(CONTACTS_PB);
							contacts_changed = true;
						}
						

					} else if (res2 == gui2::tedit_group::OCR) {
						std::vector<std::string> fields;
						for (std::vector<std::pair<std::string, std::string> >::const_iterator it = parameters.map.begin(); it != parameters.map.end(); ++ it) {
							fields.push_back(it->second);
						}
						const int proto_fields = group.fields_size();
						for (int at = 0; at < proto_fields; at ++) {
							const std::string& field_id = group.fields(at);
							const pb::tbh_field* bh_field = find_bh_field_from_contacts(contacts, field_id);
							if (bh_field) {
								const std::string& name = bh_field->name();
								fields.push_back(name);
							}
						}
						game.handle_ocr(fields);
						res2 = gui2::tedit_group::EDIT_PERSON;

					} else if (res2 == gui2::twindow::OK) {
						bool require_write_pb = false;
						if (contacts_changed) {
							VALIDATE(dirty, null_str);

							if (!is_group_equal(contacts.groups(this_group_at), group)) {
								// fields except persons was changed. for example group's name.
								*contacts.mutable_groups(this_group_at) = group;
								require_write_pb = true;
							}

						} else {
							if (context_result == nposm) {
								game.insert_group(group);
								dirty = true;
							} else {
								if (!is_group_equal(contacts.groups(context_result), group)) {
									*contacts.mutable_groups(context_result) = group;
									dirty = true;
								}
							}
							require_write_pb = dirty;
						}
						if (dirty) {
							if (send_info.valid() && this_group_at == 0) {
								generate_send_queue(send_info, game.send_records(), contacts, templates, game.parameters(), time(nullptr));
							}
						}
						if (require_write_pb) {
							VALIDATE(dirty, null_str);
							game.write_pb(CONTACTS_PB);
						}
					}
				}
				start_layer = gui2::thome::GROUP_LAYER;

			} else if (res == gui2::thome::EDIT_TEMPLATE) {
				const bool edit = context_result != nposm;
				pb::ttemplate template2;
				if (edit) {
					template2 = templates.templates(context_result);
				}
				const int this_template_at = context_result == nposm? templates.templates_size(): context_result;
				const bool read_only = edit && !send_info.can_edit_contacts_or_templates() && this_template_at == 0;

				int res2 = gui2::tedit_template::OCR;
				while (res2 == gui2::tedit_template::OCR) {
					{
						gui2::tedit_template dlg(contacts, templates, template2, edit, game.parameters(), read_only);
						dlg.show();
						start_layer = gui2::thome::TEMPLATE_LAYER;
						res2 = dlg.get_retval();
					}
					if (res2 == gui2::tedit_template::OCR) {
						std::vector<std::string> fields;
						fields.push_back(_("Template"));
						game.handle_ocr(fields);
					}
				}
				if (res2 != gui2::twindow::OK) {
					continue;
				}

				bool dirty = false;
				if (context_result == nposm) {
					game.insert_template(template2);
					dirty = true;
				} else {
					if (!is_template_equal(templates.templates(context_result), template2)) {
						*templates.mutable_templates(context_result) = template2;
						dirty = true;
					}
				}
				if (dirty) {
					if (send_info.valid() && this_template_at == 0) {
						generate_send_queue(send_info, game.send_records(), contacts, templates, game.parameters(), time(nullptr));
					}
					game.write_pb(TEMPLATES_PB);
				}

			} else if (res == gui2::thome::EDIT_FIELD) {
				const bool edit = context_result != nposm;
				pb::tbh_field bh_field;
				if (edit) {
					bh_field = contacts.bh_fields(context_result);
				}
				const int this_bh_field_at = context_result == nposm? contacts.bh_fields_size(): context_result;


				gui2::tedit_field dlg(game.contacts(), bh_field, edit, game.parameters());
				dlg.show();
				start_layer = gui2::thome::FIELD_LAYER;
				if (dlg.get_retval() != gui2::twindow::OK) {
					continue;
				}

				bool dirty = false;
				if (context_result == nposm) {
					game.insert_bh_field(bh_field);
					dirty = true;
				} else {
					if (!is_bh_field_equal(contacts.bh_fields(context_result), bh_field)) {
						*contacts.mutable_bh_fields(context_result) = bh_field;
						dirty = true;
					}
				}
				if (dirty) {
					game.write_pb(CONTACTS_PB);
				}

			} else if (res == gui2::thome::LOGIN) {
				game.handle_login();

			} else if (res == gui2::thome::NOTIFICATION) {
				std::string layer;
				if (context_result == gui2::thome::SMS_LAYER) {
					layer = _("SMS");
				} else if (context_result == gui2::thome::GROUP_LAYER) {
					layer = _("Group");
				} else if (context_result == gui2::thome::TEMPLATE_LAYER) {
					layer = _("Template");
				} else if (context_result == gui2::thome::FIELD_LAYER) {
					layer = _("Field");
				} else {
					VALIDATE(context_result == gui2::thome::MORE_LAYER, null_str);
					layer = _("More");
				}

				gui2::tnotification dlg(send_info, game.contacts(), game.templates(), layer);
				dlg.show();
				start_layer = dlg.get_retval() == gui2::tnotification::LOGIN? gui2::thome::MORE_LAYER: context_result;

				if (send_info.valid() && send_info.can_edit_contacts_or_templates()) {
					generate_send_queue(game.send_info(), game.send_records(), game.contacts(), game.templates(), game.parameters(), time(nullptr));
				}

			} else if (res == gui2::thome::CHANGE_LANGUAGE) {
				if (game.change_language()) {
					start_layer = gui2::thome::MORE_LAYER;
					t_string::reset_translations();
					image::flush_cache();
				}

			} else {
				start_layer = gui2::thome::SMS_LAYER;
			}
		}

	} catch (twml_exception& e) {
		e.show();

	} catch (CVideo::quit&) {
		//just means the game should quit
		SDL_Log("SDL_main, catched CVideo::quit\n");

	} catch (game_logic::formula_error& e) {
		gui2::show_error_message(e.what());
	} 

	return 0;
}

int main(int argc, char** argv)
{
	try {
		do_gameloop(argc, argv);
	} catch (twml_exception& e) {
		// this exception is generated when create instance.
		SDL_SimplerMB("%s\n", e.user_message.c_str());
	}

	return 0;
}