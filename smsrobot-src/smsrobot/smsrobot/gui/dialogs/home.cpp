#define GETTEXT_DOMAIN "smsrobot-lib"

#include "gui/dialogs/home.hpp"

#include "gui/widgets/label.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/report.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/stack.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/message2.hpp"
#include "gui/dialogs/combo_box2.hpp"
#include "gui/dialogs/edit_box.hpp"
#include "gettext.hpp"
#include "formula_string_utils.hpp"
#include "filesystem.hpp"
#include "base_instance.hpp"
#include "language.hpp"
#include "network.hpp"

#include <boost/bind.hpp>

namespace gui2 {

REGISTER_DIALOG(smsrobot, home)

thome::thome(const std::vector<tsim_info>& sim_infos, tsend_info& send_info, tsms_smart& sms_smart, std::vector<tsend_record>& send_records, pb::tcontacts& contacts, pb::ttemplates& templates, const tparameters& parameters, const boost::function<void (const bool)>& did_run_task, int start_layer)
	: sim_infos_(sim_infos)
	, send_info_(send_info)
	, sms_smart_(sms_smart)
	, send_records_(send_records)
	, contacts_(contacts)
	, templates_(templates)
	, parameters_(parameters)
	, did_run_task_(did_run_task)
	, start_layer_(start_layer)
	, reload_sms_layer_(false)
	, reload_template_(false)
{
	intervals_.insert(std::make_pair(30, _("30 Sec")));
	intervals_.insert(std::make_pair(45, _("45 Sec")));
	intervals_.insert(std::make_pair(60, _("60 Sec")));
	intervals_.insert(std::make_pair(75, _("75 Sec")));
	intervals_.insert(std::make_pair(2 * 60, _("2 Min")));
}

void thome::pre_show()
{
	window_->set_label("misc/white-background.png");

	body_ = find_widget<tstack>(window_, "body", false, true);
	pre_sms(*window_);
	pre_group(*window_);
	pre_template(*window_);
	pre_field(*window_);
	pre_more(*window_);

	treport* report = find_widget<treport>(window_, "navigation", false, true);
	tcontrol* item = &report->insert_item("sms_item", _("SMS"));
	item->set_icon("misc/sms.png");

	item = &report->insert_item(null_str, _("Group"));
	item->set_icon("misc/contacts.png");

	item = &report->insert_item(null_str, _("Template"));
	item->set_icon("misc/template.png");

	item = &report->insert_item(null_str, _("Field"));
	item->set_icon("misc/field96.png");

	item = &report->insert_item(null_str, _("More"));
	item->set_icon("misc/more.png");

	report->set_did_item_changed(boost::bind(&thome::did_body_changed, this, _1, _2));
	report->select_item(start_layer_);
}

void thome::post_show()
{
}

//
// sms
//
void thome::pre_sms(twindow& window)
{
	tgrid* sms_layer = body_->layer(SMS_LAYER);

	tbutton* button = find_widget<tbutton>(sms_layer, "notification", false, true);
	connect_signal_mouse_left_click(
			  *button
			, boost::bind(
				&thome::handle_notification
				, this, boost::ref(window)
				, SMS_LAYER));

	// sim strategy
	button = find_widget<tbutton>(window_, "sim", false, true);
	button->set_icon("misc/sim.png");
	button->set_canvas_variable("value", variant(sim_strategy_short_description(send_info_.sim_strategy)));

	connect_signal_mouse_left_click(
			  *button
			, boost::bind(
				&thome::select_sim_strategy
				, this, boost::ref(window)));

	// interval
	button = find_widget<tbutton>(window_, "interval", false, true);
	button->set_icon("misc/interval.png");
	std::map<int, std::string>::iterator interval_it = intervals_.find(send_info_.interval);
	if (interval_it == intervals_.end()) {
		send_info_.interval = 60;
		interval_it = intervals_.find(send_info_.interval);
		preferences::set_interval(send_info_.interval);
		VALIDATE(interval_it != intervals_.end(), null_str);
	}
	button->set_canvas_variable("value", variant(interval_it->second));

	tlistbox* history = find_widget<tlistbox>(sms_layer, "history", false, true);
	history->enable_select(false);

	connect_signal_mouse_left_click(
			  *button
			, boost::bind(
				&thome::select_interval
				, this, boost::ref(window)));

	button = find_widget<tbutton>(sms_layer, "run_task", false, true);
	button->set_label(send_info_.running? "misc/stop.png": "misc/start.png");
	connect_signal_mouse_left_click(
			  *button
			, boost::bind(
				&thome::run_task
				, this, boost::ref(window)));

	button = find_widget<tbutton>(sms_layer, "reset_task", false, true);
	connect_signal_mouse_left_click(
			  *button
			, boost::bind(
				&thome::reset_task
				, this, boost::ref(window)));

	// render all recorder.
	did_refresh_task_buttons();
	if (send_info_.valid()) {
		did_refresh_queue_list(true);
	}
}

void thome::reload_sms_layer()
{
	treport* report = find_widget<treport>(window_, "navigation", false, true);
	VALIDATE(report->cursel()->at() == SMS_LAYER, null_str);

	generate_send_queue(send_info_, send_records_, contacts_, templates_, parameters_, time(nullptr));

	tgrid* sms_layer = body_->layer(SMS_LAYER);
	// render all recorder.
	did_refresh_task_buttons();
	find_widget<tlistbox>(sms_layer, "history", false, true)->clear();
	if (send_info_.valid()) {
		did_refresh_queue_list(true);
	}
}

void thome::select_sim_strategy(twindow& window)
{
	if (!send_info_.can_edit_contacts_or_templates()) {
		return;
	}

	std::vector<std::string> items;
	std::vector<int> values;
	int sim_strategy = sim_none;
	
	for (std::vector<tsim_info>::const_iterator it = sim_infos_.begin(); it != sim_infos_.end(); ++ it) {
		const tsim_info& sim = *it;
		if (sim.slot == 0) {
			items.push_back(_("SIM1 always"));
			values.push_back(sim_sim0);

		} else if (sim.slot == 1) {
			items.push_back(_("SIM2 always"));
			values.push_back(sim_sim1);

		} else {
			VALIDATE(false, null_str);
		}
	}
	if (sim_infos_.size() == 2) {
		items.push_back(_("Alternate use SIM1 and SIM2"));
		values.push_back(sim_alternate);
	}

	int at = 0;
	for (std::vector<int>::const_iterator it = values.begin(); it != values.end(); ++ it, at ++) {
		int val = *it;
		if (send_info_.sim_strategy == val) {
			sim_strategy = at;
		}
	}

	gui2::tcombo_box2 dlg(_("Select SIM strategy"), items, sim_strategy);
	dlg.show();
	if (dlg.get_retval() != twindow::OK) {
		return;
	}

	sim_strategy = values[dlg.cursel()];
	if (sim_strategy != send_info_.sim_strategy) {
		preferences::set_sim_strategy(sim_strategy);

		send_info_.sim_strategy = sim_strategy;
		tbutton* sim = find_widget<tbutton>(window_, "sim", false, true);
		sim->set_canvas_variable("value", variant(sim_strategy_short_description(send_info_.sim_strategy)));

		reload_sms_layer();
	}
}

void thome::select_interval(twindow& window)
{
	if (!send_info_.can_edit_contacts_or_templates()) {
		return;
	}

	std::vector<std::string> items;
	std::vector<int> values;
	int initial_sel = nposm;

	for (std::map<int, std::string>::const_iterator it = intervals_.begin(); it != intervals_.end(); ++ it) {
		items.push_back(it->second);
		values.push_back(it->first);
		if (values.back() == send_info_.interval) {
			initial_sel = values.size() - 1;
		}
	}

	gui2::tcombo_box2 dlg(_("Select send interval"), items, initial_sel);
	dlg.show();
	if (dlg.get_retval() != twindow::OK) {
		return;
	}

	int interval = values[dlg.cursel()];
	if (interval != send_info_.interval) {
		preferences::set_interval(interval);

		send_info_.interval = interval;
		tbutton* interval = find_widget<tbutton>(window_, "interval", false, true);
		interval->set_canvas_variable("value", variant(items[dlg.cursel()]));

		reload_sms_layer();
	}
}

void thome::run_task(twindow& window)
{
	bool startup = !send_info_.running;
	SDL_Log("thome::run_task, will call did_run_task_(%s)\n", startup? "true": "false");
	did_run_task_(startup);

	did_refresh_task_buttons();
}

void thome::did_refresh_task_buttons()
{
	tgrid* sms_layer = body_->layer(SMS_LAYER);
	twidget::tvisible visible = twidget::VISIBLE;

	if (!send_info_.valid() || !group_and_template_ready()) {
		find_widget<tgrid>(sms_layer, "invalid_grid", false).set_visible(twidget::VISIBLE);
		find_widget<tgrid>(sms_layer, "valid_grid", false).set_visible(twidget::INVISIBLE);

		tlabel& label = find_widget<tlabel>(sms_layer, "invalid_description", false);
		std::string description;
		if (!send_info_.valid()) {
			description = _("No valid SIM strategy was selected, and cannot generate send queues.");
		} else {
			description = _("The first group or the first template is not valid, cannot generate send queues.");
		}
		label.set_label(description);
		return;
	}
	
	find_widget<tgrid>(sms_layer, "invalid_grid", false).set_visible(twidget::INVISIBLE);
	find_widget<tgrid>(sms_layer, "valid_grid", false).set_visible(twidget::VISIBLE);

	tbutton* button = find_widget<tbutton>(sms_layer, "run_task", false, true);
	button->set_label(send_info_.running? "misc/stop.png": "misc/start.png");
	if (send_info_.valid()) {
		if (!send_info_.running && send_info_.next_send_at == (int)send_records_.size()) {
			visible = twidget::HIDDEN;
		} else {
			visible = twidget::VISIBLE;
		}
	} else {
		visible = twidget::HIDDEN;
	}
	button->set_visible(visible);

	// update reset_task status
	button = find_widget<tbutton>(sms_layer, "reset_task", false, true);
	if (send_info_.valid()) {
		if (!send_info_.running && send_info_.next_send_at) {
			visible = twidget::VISIBLE;
		} else {
			visible = twidget::HIDDEN;
		}
	} else {
		visible = twidget::HIDDEN;
	}
	button->set_visible(visible);
}

void thome::reset_task(twindow& window)
{
	tgrid* sms_layer = body_->layer(SMS_LAYER);

	VALIDATE(!send_info_.running && send_info_.valid() && send_info_.next_send_at, null_str);
	send_info_.next_send_at = 0;
	preferences::set_next_send_at(send_info_.next_send_at);
	did_refresh_queue_list(false);

	tbutton* button = find_widget<tbutton>(sms_layer, "reset_task", false, true);
	button->set_visible(twidget::HIDDEN);

	button = find_widget<tbutton>(sms_layer, "run_task", false, true);
	button->set_visible(twidget::VISIBLE);
}

void thome::did_refresh_queue_list(bool erase)
{
	threading::lock lock(history_mutex_);
	tgrid* sms_layer = body_->layer(SMS_LAYER);
	const int records = send_records_.size();

	tlistbox* history = find_widget<tlistbox>(sms_layer, "history", false, true);
	if (erase) {
		history->clear();
	} else {
		VALIDATE(history->rows() == records, null_str);
	}

	if (send_records_.empty()) {
		return;
	}

	std::map<std::string, std::string> data;
	std::stringstream ss;
	for (int at = 0; at < records; at ++) {
		const tsend_record& record = send_records_[at];
		std::string time_str = at < send_info_.next_send_at? format_time_hms(record.t): ht::generate_format(format_time_hms(record.t), 0xffff0000);
		if (erase) {
			data["time"] = time_str;
			ss.str("");
			ss << "SIM#" + str_cast(record.slot + 1) << "->\n" << record.receiver;
			data["sender"] = ss.str();
			data["message"] = record.message;
	
			ttoggle_panel& row = history->insert_row(data);
			row.set_cookie(record.t);
		} else {
			ttoggle_panel& row = history->row_panel(at);
			tlabel* label = dynamic_cast<tlabel*>(row.find("time", false));
			label->set_label(time_str);
		}
	}
}

void thome::did_sms_sent(const int at)
{
	threading::lock lock(history_mutex_);

	tgrid* sms_layer = body_->layer(SMS_LAYER);
	tlistbox* history = find_widget<tlistbox>(sms_layer, "history", false, true);

	VALIDATE(at < (int)send_records_.size(), null_str);
	
	const tsend_record& record = send_records_[at];

	ttoggle_panel& row = history->row_panel(at);
	tlabel* label = dynamic_cast<tlabel*>(row.find("time", false));
	label->set_label(format_time_hms(record.t));	

	int desire_at = at + 1 != (int)send_records_.size()? at + 1: at;
	history->scroll_to_row(desire_at);
}

//
// group
//
void thome::pre_group(twindow& window)
{
	tgrid* group_layer = body_->layer(GROUP_LAYER);

	tbutton* button = find_widget<tbutton>(group_layer, "notification", false, true);
	connect_signal_mouse_left_click(
			  *button
			, boost::bind(
				&thome::handle_notification
				, this, boost::ref(window)
				, GROUP_LAYER));
	//
	// groups
	//
	tlistbox* groups = find_widget<tlistbox>(window_, "groups", false, true);
	groups->enable_select(false);
	fill_groups_listbox(*groups);
	groups->set_did_can_drag(boost::bind(&thome::did_group_can_drag, this, _1, _2));
#ifdef _WIN32
	groups->set_did_row_double_click(boost::bind(&thome::did_edit_group, this, _2));
#else
	groups->set_did_row_changed(boost::bind(&thome::did_edit_group, this, _2));
#endif

	button = find_widget<tbutton>(group_layer, "insert_group", false, true);
	connect_signal_mouse_left_click(
			  *button
			, boost::bind(
				&thome::new_group
				, this, boost::ref(window)));

	button = dynamic_cast<tbutton*>(groups->left_drag_grid()->find("send", true));
	button->set_icon("misc/red-background.png");
	connect_signal_mouse_left_click(
		*button
		, boost::bind(
			&thome::require_send_group
			, this
			, boost::ref(*groups)));

	button = dynamic_cast<tbutton*>(groups->left_drag_grid()->find("clone", true));
	button->set_icon("misc/decorate-background.png");
	connect_signal_mouse_left_click(
		*button
		, boost::bind(
			&thome::clone_group
			, this
			, boost::ref(*groups)));

	button = dynamic_cast<tbutton*>(groups->left_drag_grid()->find("erase", true));
	button->set_icon("misc/red-background.png");
	connect_signal_mouse_left_click(
		*button
		, boost::bind(
			&thome::erase_group
			, this
			, boost::ref(*groups)));
}

static std::string generate_group_description(const pb::tgroup& group)
{
	int mobiles = 0;
	for (int at = 0; at < group.persons_size(); at ++) {
		const pb::tperson& person = group.persons(at);
		mobiles += person.mobiles_size();
	}

	utils::string_map symbols;
	std::stringstream ss;

	symbols["count"] = str_cast(group.persons_size());
	ss << vgettext2("$count contacts", symbols);
	ss << "\n";
	symbols["count"] = str_cast(mobiles);
	ss << vgettext2("$count mobiles", symbols);
	return ss.str();
}

void thome::fill_groups_listbox(tlistbox& listbox)
{
	utils::string_map symbols;
	listbox.clear();

	std::map<std::string, std::string> data;
	const int proto_groups = contacts_.groups_size();
	for (int at = 0; at < proto_groups; at ++) {
		const pb::tgroup& proto_group = contacts_.groups(at);

		data["name"] = proto_group.name();
		data["persons"] = generate_group_description(proto_group);

		ttoggle_panel& row = listbox.insert_row(data);
		post_group_row_ui(row);
	}
}

void thome::post_group_row_ui(ttoggle_panel& row)
{
	const int at = row.at();

	tcontrol& panel = *dynamic_cast<tcontrol*>(row.find("panel", false));
	if (at || send_info_.can_edit_contacts_or_templates()) {
		panel.set_label(null_str);
	} else {
		panel.set_label("misc/red-translucent10-background.png");
	}
	

	std::vector<gui2::tformula_blit> blits;
	tcontrol& portrait = *dynamic_cast<tcontrol*>(row.find("portrait", false));
	blits.push_back(gui2::tformula_blit("misc/group-color96.png", null_str, null_str, "(width)", "(height)"));
	if (!at) {
		blits.push_back(gui2::tformula_blit("misc/send.png", null_str, null_str, "(width / 2)", "(width / 2)"));
	}
	portrait.set_blits(blits);
}

void thome::require_send_group(tlistbox& listbox)
{
	const int drag_at = listbox.drag_at();
	VALIDATE(drag_at > 0, null_str);

	// post handle
	// swap drag_a to 0.
	const int to_at = 0;
	for (int at = to_at; at < drag_at; at ++) {
		contacts_.mutable_groups()->SwapElements(at, drag_at);
	}

	fill_groups_listbox(listbox);

	reload_sms_layer_ = true;

	instance->write_pb(CONTACTS_PB);
}

void thome::clone_group(tlistbox& listbox)
{
	const int drag_at = listbox.drag_at();

	pb::tgroup* new_group = contacts_.add_groups();
	*new_group = contacts_.groups(drag_at);
	new_group->set_name(new_group->name() + "-" + _("file^Clone"));

	std::map<std::string, std::string> data;
	data["name"] = new_group->name();
	data["persons"] = generate_group_description(*new_group);

	ttoggle_panel& row = listbox.insert_row(data);
	post_group_row_ui(row);

	instance->write_pb(CONTACTS_PB);
}

void thome::erase_group(tlistbox& listbox)
{
	const int drag_at = listbox.drag_at();

	std::stringstream ss;
	utils::string_map symbols;
	symbols["group"] = contacts_.groups(drag_at).name();
	ss << vgettext2("Do you want to delete group: $group|?", symbols);

	const int res = gui2::show_message2(ss.str(), _("Delete"));
	if (res == gui2::twindow::CANCEL) {
		return;
	}

	// post handle
	contacts_.mutable_groups()->DeleteSubrange(drag_at, 1);
	if (drag_at != listbox.rows() - 1) {
		fill_groups_listbox(listbox);

	} else {
		listbox.erase_row(drag_at);
	}

	if (!drag_at) {
		reload_sms_layer_ = true;
	}

	instance->write_pb(CONTACTS_PB);
}

void thome::new_group(twindow& window)
{
	context_result_ = nposm;
	window.set_retval(EDIT_GROUP);
}

bool thome::did_group_can_drag(tlistbox& listbox, ttoggle_panel& row)
{
	const int at = row.at();
	std::map<std::string, twidget::tvisible> visibles;
	visibles.insert(std::make_pair("send", at && send_info_.can_edit_contacts_or_templates()? twidget::VISIBLE: twidget::INVISIBLE));
	visibles.insert(std::make_pair("erase", at || send_info_.can_edit_contacts_or_templates()? twidget::VISIBLE: twidget::INVISIBLE));

	listbox.left_drag_grid_set_widget_visible(visibles);
	return true;
}

void thome::did_edit_group(ttoggle_panel& row)
{
	context_result_ = row.at();
	window_->set_retval(EDIT_GROUP);
}

//
// template
//
void thome::pre_template(twindow& window)
{
	tgrid* template_layer = body_->layer(TEMPLATE_LAYER);

	tbutton* button = find_widget<tbutton>(template_layer, "notification", false, true);
	connect_signal_mouse_left_click(
			  *button
			, boost::bind(
				&thome::handle_notification
				, this, boost::ref(window)
				, TEMPLATE_LAYER));

	//
	// templates
	//
	tlistbox* templates = find_widget<tlistbox>(template_layer, "templates", false, true);
	templates->enable_select(false);
	fill_templates_listbox(*templates);
	templates->set_did_can_drag(boost::bind(&thome::did_template_can_drag, this, _1, _2));
#ifdef _WIN32
	templates->set_did_row_double_click(boost::bind(&thome::did_edit_template, this, _2));
#else
	templates->set_did_row_changed(boost::bind(&thome::did_edit_template, this, _2));
#endif


	button = find_widget<tbutton>(template_layer, "insert", false, true);
	connect_signal_mouse_left_click(
			  *button
			, boost::bind(
				&thome::new_template
				, this, boost::ref(window)));

	button = dynamic_cast<tbutton*>(templates->left_drag_grid()->find("send", true));
	button->set_icon("misc/red-background.png");
	connect_signal_mouse_left_click(
		*button
		, boost::bind(
			&thome::require_send_template
			, this
			, boost::ref(*templates)));

	button = dynamic_cast<tbutton*>(templates->left_drag_grid()->find("clone", true));
	button->set_icon("misc/decorate-background.png");
	connect_signal_mouse_left_click(
		*button
		, boost::bind(
			&thome::clone_template
			, this
			, boost::ref(*templates)));

	button = dynamic_cast<tbutton*>(templates->left_drag_grid()->find("erase", true));
	button->set_icon("misc/red-background.png");
	connect_signal_mouse_left_click(
		*button
		, boost::bind(
			&thome::erase_template
			, this
			, boost::ref(window)
			, boost::ref(*templates)));
}

void thome::fill_templates_listbox(tlistbox& listbox)
{
	listbox.clear();
	std::map<std::string, std::string> data;
	const int proto_templates = templates_.templates_size();
	for (int at = 0; at < proto_templates; at ++) {
		const pb::ttemplate& proto_template = templates_.templates(at);

		data["name"] = proto_template.name();
		data["message"] = template_generate_preview(parameters_, contacts_, proto_template);		

		ttoggle_panel& row = listbox.insert_row(data);
		post_template_row_ui(row);
	}
}

void thome::post_template_row_ui(ttoggle_panel& row)
{
	const int at = row.at();

	tcontrol& panel = *dynamic_cast<tcontrol*>(row.find("panel", false));
	if (at || send_info_.can_edit_contacts_or_templates()) {
		panel.set_label(null_str);
	} else {
		panel.set_label("misc/red-translucent10-background.png");
	}

	std::vector<gui2::tformula_blit> blits;
	tcontrol& portrait = *dynamic_cast<tcontrol*>(row.find("portrait", false));
	blits.push_back(gui2::tformula_blit("misc/message-color96.png", null_str, null_str, "(width)", "(height)"));
	if (!at) {
		blits.push_back(gui2::tformula_blit("misc/send.png", null_str, null_str, "(width / 2)", "(width / 2)"));
	}
	portrait.set_blits(blits);
}

void thome::new_template(twindow& window)
{
	context_result_ = nposm;
	window.set_retval(EDIT_TEMPLATE);
}

void thome::require_send_template(tlistbox& listbox)
{
	const int drag_at = listbox.drag_at();
	VALIDATE(drag_at > 0, null_str);

	// post handle
	// swap drag_a to 0.
	const int to_at = 0;
	for (int at = to_at; at < drag_at; at ++) {
		templates_.mutable_templates()->SwapElements(at, drag_at);
	}

	fill_templates_listbox(listbox);

	reload_sms_layer_ = true;

	instance->write_pb(TEMPLATES_PB);
}

void thome::clone_template(tlistbox& listbox)
{
	const int drag_at = listbox.drag_at();

	pb::ttemplate* new_template = templates_.add_templates();
	*new_template = templates_.templates(drag_at);
	new_template->set_name(new_template->name() + "-" + _("file^Clone"));

	std::map<std::string, std::string> data;
	data["name"] = new_template->name();
	data["message"] = template_generate_preview(parameters_, contacts_, *new_template);

	ttoggle_panel& row = listbox.insert_row(data);
	post_template_row_ui(row);

	instance->write_pb(TEMPLATES_PB);
}

void thome::erase_template(twindow& window, tlistbox& listbox)
{
	tgrid* template_layer = body_->layer(TEMPLATE_LAYER);
	int drag_at = listbox.drag_at();
	const pb::ttemplate& proto_template = templates_.templates(drag_at);

	std::stringstream ss;
	utils::string_map symbols;
	symbols["template"] = proto_template.name();
	ss << vgettext2("Do you want to delete template: $template|?", symbols);

	const int res = gui2::show_message2(ss.str(), _("Delete"));
	if (res == gui2::twindow::CANCEL) {
		return;
	}

	// post handle
	templates_.mutable_templates()->DeleteSubrange(drag_at, 1);
	if (drag_at != listbox.rows() - 1) {
		fill_templates_listbox(listbox);

	} else {
		listbox.erase_row(drag_at);
	}

	if (!drag_at) {
		reload_sms_layer_ = true;
	}
	
	instance->write_pb(TEMPLATES_PB);
}

bool thome::did_template_can_drag(tlistbox& listbox, ttoggle_panel& row)
{
	const int at = row.at();
	std::map<std::string, twidget::tvisible> visibles;
	visibles.insert(std::make_pair("send", at && send_info_.can_edit_contacts_or_templates()? twidget::VISIBLE: twidget::INVISIBLE));
	visibles.insert(std::make_pair("erase", at || send_info_.can_edit_contacts_or_templates()? twidget::VISIBLE: twidget::INVISIBLE));

	listbox.left_drag_grid_set_widget_visible(visibles);
	return true;
}

void thome::did_edit_template(ttoggle_panel& row)
{
	context_result_ = row.at();
	window_->set_retval(EDIT_TEMPLATE);
}

// 
// field
//
void thome::pre_field(twindow& window)
{
	tgrid* field_layer = body_->layer(FIELD_LAYER);

	tbutton* button = find_widget<tbutton>(field_layer, "notification", false, true);
	connect_signal_mouse_left_click(
			  *button
			, boost::bind(
				&thome::handle_notification
				, this, boost::ref(window)
				, FIELD_LAYER));

	//
	// fields
	//
	tlistbox* fields = find_widget<tlistbox>(field_layer, "fields", false, true);
	fields->enable_select(false);
	fill_fields_listbox(*fields);
	fields->set_did_can_drag(boost::bind(&thome::did_fields_can_drag, this, _2));
#ifdef _WIN32
	fields->set_did_row_double_click(boost::bind(&thome::did_edit_field, this, _2));
#else
	fields->set_did_row_changed(boost::bind(&thome::did_edit_field, this, _2));
#endif


	button = find_widget<tbutton>(field_layer, "insert", false, true);
	connect_signal_mouse_left_click(
			  *button
			, boost::bind(
				&thome::new_field
				, this, boost::ref(window)));

	button = dynamic_cast<tbutton*>(fields->left_drag_grid()->find("erase", true));
	button->set_icon("misc/red-background.png");
	connect_signal_mouse_left_click(
		*button
		, boost::bind(
			&thome::erase_field
			, this
			, boost::ref(window)
			, boost::ref(*fields)));
}

void thome::fill_fields_listbox(tlistbox& listbox)
{
	listbox.clear();
	std::map<std::string, std::string> data;

	for (std::vector<std::pair<std::string, std::string> >::const_iterator it = parameters_.map.begin(); it != parameters_.map.end(); ++ it) {
		data["id"] = it->first;
		data["name"] = it->second;
	
		ttoggle_panel& row = listbox.insert_row(data);
		post_field_row_ui(row, true, it->first);
	}

	const int proto_bh_fields = contacts_.bh_fields_size();
	for (int at = 0; at < proto_bh_fields; at ++) {
		const pb::tbh_field& proto_bh_field = contacts_.bh_fields(at);

		data["id"] = proto_bh_field.id();
		data["name"] = proto_bh_field.name();
	
		ttoggle_panel& row = listbox.insert_row(data);
		post_field_row_ui(row, false, proto_bh_field.id());
	}
}

void thome::post_field_row_ui(ttoggle_panel& row, bool fixed, const std::string& field_id)
{
	const int at = row.at();

	tcontrol& panel = *dynamic_cast<tcontrol*>(row.find("panel", false));
	if (!fixed && !send_info_.can_edit_contacts_or_templates() && field_in_first_template(field_id)) {
		panel.set_label("misc/red-translucent10-background.png");
	} else {
		panel.set_label(null_str);
	}
	

	std::vector<gui2::tformula_blit> blits;
	tcontrol& portrait = *dynamic_cast<tcontrol*>(row.find("portrait", false));
	blits.push_back(gui2::tformula_blit("misc/field96.png", null_str, null_str, "(width)", "(height)"));
	if (fixed) {
		blits.push_back(gui2::tformula_blit("misc/lock.png", null_str, null_str, "(width / 2)", "(width / 2)"));
	}
	portrait.set_blits(blits);
}

void thome::new_field(twindow& window)
{
	context_result_ = nposm;
	window.set_retval(EDIT_FIELD);
}

void thome::erase_field(twindow& window, tlistbox& listbox)
{
	tgrid* field_layer = body_->layer(FIELD_LAYER);
	const int drag_at = listbox.drag_at();
	const int bh_fields_at = drag_at - parameters_.map.size();
	const pb::tbh_field proto_bh_field = contacts_.bh_fields(bh_fields_at);

	std::stringstream ss;
	utils::string_map symbols;
	symbols["field"] = proto_bh_field.name();
	ss << vgettext2("Do you want to delete field: $field|?", symbols);

	const int res = gui2::show_message2(ss.str(), _("Delete"));
	if (res == gui2::twindow::CANCEL) {
		return;
	}

	// post handle
	contacts_.mutable_bh_fields()->DeleteSubrange(bh_fields_at, 1);
	if (drag_at != listbox.rows() - 1) {
		fill_fields_listbox(listbox);

	} else {
		listbox.erase_row(drag_at);
	}

	VALIDATE(send_info_.can_edit_contacts_or_templates() || !field_in_first_template(proto_bh_field.id()), null_str);

	reload_template_ = true;

	instance->write_pb(CONTACTS_PB);
}

void thome::did_edit_field(ttoggle_panel& row)
{
	if (row.at() < (int)parameters_.map.size()) {
		return;
	}
	if (!send_info_.can_edit_contacts_or_templates()) {
		const int field_at = row.at() - parameters_.map.size();
		if (field_in_first_template(contacts_.bh_fields(field_at).id())) {
			return;
		}
	}

	context_result_ = row.at() - parameters_.map.size();
	window_->set_retval(EDIT_FIELD);
}

bool thome::field_in_first_template(const std::string& id) const
{
	VALIDATE(!id.empty(), null_str);

	if (templates_.templates_size() == 0) {
		return false;
	}
	const pb::ttemplate& template2 = templates_.templates(0);
	int params = template2.params_size();
	for (int at = 0; at < params; at ++) {
		if (template2.params(at).field_id() == id) {
			return true;
		}
	}
	return false;
}

bool thome::did_fields_can_drag(ttoggle_panel& row) const
{
	if (row.at() < (int)parameters_.map.size()) {
		return false;
	}
	if (!send_info_.can_edit_contacts_or_templates()) {
		const int field_at = row.at() - parameters_.map.size();
		if (field_in_first_template(contacts_.bh_fields(field_at).id())) {
			return false;
		}
	}

	return true;
}

// 
// more
//
void thome::pre_more(twindow& window)
{
	tgrid* more_layer = body_->layer(MORE_LAYER);

	tbutton* button = find_widget<tbutton>(more_layer, "notification", false, true);
	connect_signal_mouse_left_click(
			  *button
			, boost::bind(
				&thome::handle_notification
				, this, boost::ref(window)
				, MORE_LAYER));

	update_relative_login(window);

	connect_signal_mouse_left_click(
			find_widget<tbutton>(more_layer, "login", false)
			, boost::bind(
				&thome::set_retval
				, this
				, boost::ref(window)
				, (int)LOGIN));

	button = find_widget<tbutton>(more_layer, "logout", false, true);
	button->set_icon("misc/logout.png");
	button->set_canvas_variable("value", variant(game_config::bbs_server.generate_furl()));
	connect_signal_mouse_left_click(
			*button
			, boost::bind(
				&thome::logout
				, this
				, boost::ref(window)));

	tbutton* language = find_widget<tbutton>(more_layer, "language", false, true);
	language->set_icon("misc/sms-selected.png");
	const language_def& current_language = get_language();
	language->set_canvas_variable("value", variant(current_language.language));

	connect_signal_mouse_left_click(
		*language
		, boost::bind(
			&thome::set_retval
			, this
			, boost::ref(window)
			, (int)CHANGE_LANGUAGE));

	// bbs server
	button = find_widget<tbutton>(more_layer, "bbs_server", false, true);
	button->set_icon("misc/sms-selected.png");
	button->set_canvas_variable("value", variant(game_config::bbs_server.generate_furl()));
	connect_signal_mouse_left_click(
		*button
		, boost::bind(
			&thome::set_bbs_url
			, this
			, boost::ref(window)));
	if (current_user.valid()) {
		button->set_visible(twidget::INVISIBLE);
	}
}

void thome::update_relative_login(twindow& window)
{
	tgrid* more_layer = body_->layer(MORE_LAYER);

	if (current_user.valid()) {
		find_widget<tgrid>(more_layer, "unlogin_grid", false, true)->set_visible(twidget::INVISIBLE);
		find_widget<tgrid>(more_layer, "logined_grid", false, true)->set_visible(twidget::VISIBLE);
		find_widget<tlabel>(more_layer, "nickname", false, true)->set_label(current_user.nick);
		find_widget<tbutton>(more_layer, "logout", false, true)->set_visible(twidget::VISIBLE);

	} else {
		find_widget<tgrid>(more_layer, "unlogin_grid", false, true)->set_visible(twidget::VISIBLE);
		find_widget<tgrid>(more_layer, "logined_grid", false, true)->set_visible(twidget::INVISIBLE);
		find_widget<tbutton>(more_layer, "logout", false, true)->set_visible(twidget::INVISIBLE);
	}
}

void thome::set_bbs_url(twindow& window)
{
	const std::string current_url = game_config::bbs_server.generate_furl();
	std::string remark_ss = _("Current url: ") + current_url;
	gui2::tedit_box::tparam param(_("Edit bbs url"), remark_ss, _("bbs url"), current_url, null_str, 30);
	// param.text_changed = boost::bind(&tmore::verify_new_nick, this, _1);
	gui2::tedit_box dlg(param);
	dlg.show();

	int res = dlg.get_retval();
	if (res != gui2::twindow::OK) {
		return;
	}
	game_config::bbs_server.split_furl(param.result, nullptr);
	preferences::set_bbs_server_furl(game_config::bbs_server.generate_furl());

	tgrid* more_layer = body_->layer(MORE_LAYER);
	tbutton* button = find_widget<tbutton>(more_layer, "bbs_server", false, true);
	button->set_canvas_variable("value", variant(game_config::bbs_server.generate_furl()));
}

void thome::logout(twindow& window)
{
	VALIDATE(current_user.valid(), null_str);

	const int res = gui2::show_message2(_("Once logout, app will not automatically login next time."), _("Logout"));
	if (res == gui2::twindow::CANCEL) {
		return;
	}

	http_agent agent;
	bool ok = agent.do_logout(current_user.uid, current_user.sessionid);
	if (ok || !current_user.valid()) {
		current_user.uid = nposm;

		preferences::set_startup_login(false);

		{
			window.set_retval(LOGIN);
			return;
		}

		update_relative_login(window);		
		refresh_notification_widget(MORE_LAYER);

		{
			tgrid* more_layer = body_->layer(MORE_LAYER);
			tbutton* button = find_widget<tbutton>(more_layer, "bbs_server", false, true);
			button->set_visible(twidget::VISIBLE);
		}
	}
}

void thome::set_retval(twindow& window, int retval)
{
	window.set_retval(retval);
}

bool thome::group_and_template_ready() const
{
	// only calulcate contacts and templates, exclue send_info_.
	if (!contacts_.groups_size() || !templates_.templates_size()) {
		return false;
	}

	const pb::tgroup& current_group = contacts_.groups(0);
	const pb::ttemplate& current_template = templates_.templates(0);

	const int persons = current_group.persons_size();
	if (!persons || current_template.message().empty()) {
		return false;
	}

	int mobiles = 0;
	for (int at = 0; at < persons; at ++) {
		const pb::tperson& person = current_group.persons(at);
		mobiles += person.mobiles_size();
	}

	return mobiles > 0? true: false;
}

bool thome::has_notification() const
{
	if (!current_user.valid()) {
		return true;
	}
	if (current_user.contacts_ts != contacts_.timestamp()) {
		return true;
	}
	if (current_user.templates_ts != templates_.timestamp()) {
		return true;
	}
	return false;
}

void thome::refresh_notification_widget(int layer)
{
	if (layer == nposm) {
		treport* report = find_widget<treport>(window_, "navigation", false, true);
		layer = report->cursel()->at();
	}
	tgrid* current_layer = body_->layer(layer);
	tbutton& widget = find_widget<tbutton>(current_layer, "notification", false);

	std::vector<gui2::tformula_blit> blits;
	blits.push_back(gui2::tformula_blit("misc/notification.png", null_str, null_str, "(width)", "(height)"));
	if (has_notification()) {
		blits.push_back(gui2::tformula_blit("misc/red-dot.png", "(width * 3 / 4)", null_str, "(width / 4)", "(width / 4)"));
	}
	widget.set_blits(blits);
}

void thome::handle_notification(twindow& window, int layer)
{
	context_result_ = layer;
	set_retval(window, NOTIFICATION);
}

void thome::did_body_changed(treport& report, ttoggle_button& widget)
{
	tgrid* current_layer = body_->layer(widget.at());
	body_->set_radio_layer(widget.at());

	tbutton* button = find_widget<tbutton>(current_layer, "notification", false, true);
	refresh_notification_widget(widget.at());

	std::string ss;
	if (widget.at() == SMS_LAYER) {
		if (reload_sms_layer_) {
			reload_sms_layer();

			reload_sms_layer_ = false;
		}

	} else if (widget.at() == GROUP_LAYER) {
		// const int max_groups = 5; 
		// find_widget<tbutton>(current_layer, "insert_group", false).set_visible(contacts_.groups_size() < max_groups? twidget::VISIBLE: twidget::HIDDEN);

		tlistbox* groups = find_widget<tlistbox>(current_layer, "groups", false, true);
		if (groups->rows()) {
			// sms layer maybe start/stop run.
			post_group_row_ui(groups->row_panel(0));
		}

	} else if (widget.at() == TEMPLATE_LAYER) {
		tlistbox* templates = find_widget<tlistbox>(current_layer, "templates", false, true);
		if (reload_template_) {
			fill_templates_listbox(*templates);

		} else if (templates->rows()) {
			// sms layer maybe start/stop run.
			post_template_row_ui(templates->row_panel(0));
		}

	} else if (widget.at() == FIELD_LAYER) {
		tlistbox* fields = find_widget<tlistbox>(current_layer, "fields", false, true);
		VALIDATE(fields->rows() == contacts_.bh_fields_size() + parameters_.map.size(), null_str);
		int bh_fields = contacts_.bh_fields_size();
		for (int at = 0; at < bh_fields; at ++) {
			const std::string& field_id = contacts_.bh_fields(at).id();
			if (field_in_first_template(field_id)) {
				post_field_row_ui(fields->row_panel(parameters_.map.size() + at), false, field_id);
			}
		}
	}
}

} // namespace gui2

