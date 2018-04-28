#define GETTEXT_DOMAIN "smsrobot-lib"

#include "gui/dialogs/edit_template.hpp"

#include "gui/widgets/label.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/scroll_text_box.hpp"
#include "gui/dialogs/combo_box2.hpp"
#include "gettext.hpp"
#include "filesystem.hpp"

#include <boost/bind.hpp>
#include <boost/regex.hpp>

#include <iomanip>

namespace gui2 {

REGISTER_DIALOG(smsrobot, edit_template)

tedit_template::tedit_template(const pb::tcontacts& contacts, const pb::ttemplates& templates, pb::ttemplate& template2, bool edit, const tparameters& parameters, bool read_only)
	: contacts_(contacts)
	, temp_template_(template2)
	, edit_(edit)
	, parameters_(parameters)
	, read_only_(read_only)
{
	if (!edit_) {
		VALIDATE(!read_only, null_str);
		temp_template_.set_name(_("Untitled template"));
	}
}

void tedit_template::pre_show()
{
	window_->set_label("misc/white-background.png");

	std::stringstream ss;
	ss.str("");
	if (edit_) {
		if (!read_only_) {
			ss << _("Edit tempalte");
		} else {
			ss << _("Browse tempalte") << "(" << _("Read only") << ")";
		}
	} else {
		ss << _("Insert tempalte");
	}
	find_widget<tlabel>(window_, "title", false, true)->set_label(ss.str());

	ttext_box* text_box = find_widget<ttext_box>(window_, "name", false, true);
	text_box->set_border(null_str);
	text_box->set_placeholder(_("Name"));
	text_box->set_label(temp_template_.name());
	text_box->set_did_text_changed(boost::bind(&tedit_template::did_field_changed, this));
	if (temp_template_.name().empty()) {
		window_->keyboard_capture(text_box);
	}
	if (read_only_) {
		text_box->set_active(false);
	}

	tscroll_text_box* scroll_text_box = find_widget<tscroll_text_box>(window_, "message", false, true);
	scroll_text_box->set_placeholder(_("Message"));
	scroll_text_box->set_label(template_generate_preview(parameters_, contacts_, temp_template_));
	scroll_text_box->set_did_text_changed(boost::bind(&tedit_template::did_field_changed, this));
	scroll_text_box->set_did_extra_menu(boost::bind(&tedit_template::did_message_extra_menu, this, _1, _2));
	if (!temp_template_.name().empty()) {
		window_->keyboard_capture(scroll_text_box->tb());
	}
	if (read_only_) {
		scroll_text_box->tb()->set_active(false);
	}

	connect_signal_mouse_left_click(
			find_widget<tbutton>(window_, "ocr", false)
		, boost::bind(
				&tedit_template::ocr
			, this
			, boost::ref(*window_)));

	// set float button in extra menu
	const std::string field_id = "flt_field";
	tfloat_widget* float_widget = window_->find_float_widget(field_id);
	float_widget->widget->set_border("float_widget");
	connect_signal_mouse_left_click(
			*float_widget->widget
		, boost::bind(
				&tedit_template::float_widget_click_field
			, this
			, boost::ref(*window_)));

	did_field_changed();

	tbutton* button = find_widget<tbutton>(window_, "save", false, true);
	if (!read_only_) {
		connect_signal_mouse_left_click(
			  find_widget<tbutton>(window_, "save", false)
			, boost::bind(
				&tedit_template::save
				, this, boost::ref(*window_)));
	} else {
		button->set_visible(twidget::HIDDEN);
	}
}

void tedit_template::post_show()
{
}

std::set<int> tedit_template::extract_parameters() const
{
	std::set<int> result;

	tscroll_text_box* scroll_text_box = find_widget<tscroll_text_box>(window_, "message", false, true);
	const std::string& message = scroll_text_box->label();

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

pb::ttemplate_param* tedit_template::find_param(const int index)
{
	int params = temp_template_.params_size();
	for (int at = 0; at < params; at ++) {
		if (temp_template_.params(at).index() == index) {
			return temp_template_.mutable_params(at);
		}
	}
	return nullptr;
}

void tedit_template::float_widget_click_field(twindow& window)
{
	std::vector<std::string> items;
	std::vector<int> values;
	int at = 0;
	int value = nposm;
	std::stringstream ss;
	int bh_fields = contacts_.bh_fields_size();

	for (std::vector<std::pair<std::string, std::string> >::const_iterator it = parameters_.map.begin(); it != parameters_.map.end(); ++ it) {
		ss.str("");
		ss << it->second << "(" << ht::generate_format(it->first, 0xff0000ff) << ")";
		items.push_back(ss.str());
		values.push_back(items.size());
	}

	for (int at = 0; at < bh_fields; at ++) {
		const pb::tbh_field& bh_field = contacts_.bh_fields(at);
		ss.str("");
		ss << bh_field.name() << "(" << ht::generate_format(bh_field.id(), 0xff0000ff) << ")";
		items.push_back(ss.str());
		values.push_back(items.size());
	}

	gui2::tcombo_box2 dlg(_("Insert field"), items, value);
	dlg.show();
	if (dlg.get_retval() != twindow::OK || !dlg.dirty()) {
		return;
	}

	if (value != values[dlg.cursel()]) {
		std::pair<std::string, std::string> pair;
		int cursel = dlg.cursel();
		if (cursel < (int)parameters_.map.size()) {
			pair = parameters_.map[cursel];
		} else {
			const pb::tbh_field& bh_field = contacts_.bh_fields(cursel - parameters_.map.size());
			pair = std::make_pair(bh_field.id(), bh_field.name());
		}
		tscroll_text_box* scroll_text_box = find_widget<tscroll_text_box>(window_, "message", false, true);
		scroll_text_box->insert_str(generate_highlight_template_field(pair.first, pair.second));
	}
}

bool field_id_existed(const tparameters& parameters, const pb::tcontacts& contacts, const std::string& field_id)
{
	if (field_id.empty()) {
		return false;
	}
	if (parameters.is_existed(field_id)) {
		return true;
	}
	if (find_bh_field_from_contacts(contacts, field_id)) {
		return true;
	}
	return false;
}

void tedit_template::ocr(twindow& window)
{
	window.set_retval(OCR);
}

void tedit_template::save(twindow& window)
{
	ttext_box* text_box = find_widget<ttext_box>(window_, "name", false, true);
	temp_template_.set_name(text_box->label());

	tscroll_text_box* scroll_text_box = find_widget<tscroll_text_box>(window_, "message", false, true);
	std::string label = scroll_text_box->label();

	temp_template_.clear_params();

	std::map<int, utils::tcfg_string_pair> parsed_items;
	utils::split_integrate_src(label, tintegrate::support_markups, parsed_items);

	std::set<int> fade_param = template_extract_parameters(label);
	std::map<std::string, int> used_params;
	std::stringstream ss;
	std::map<int, utils::tcfg_string_pair>::const_iterator it = parsed_items.begin();
	std::map<int, utils::tcfg_string_pair>::const_iterator before_it = it;
	int param_index = 0;
	int parsed_items_size = parsed_items.size();
	for (++ it; ; before_it = it, ++ it) {
		if (before_it->second.iscfg) {
			const std::string field_id = before_it->second.cfg[game_config::markup_cookie_key].str();
			std::map<std::string, int>::const_iterator find_it = used_params.find(field_id);
			if (find_it != used_params.end()) {
				ss << "%" << std::setfill('0') << std::setw(2) << find_it->second;
				if (it != parsed_items.end()) {
					continue;
				} else {
					break;
				}
			}

			if (field_id.empty() || !field_id_existed(parameters_, contacts_, field_id)) {
				if (it != parsed_items.end()) {
					continue;
				} else {
					break;
				}
			}

			while (fade_param.find(param_index) != fade_param.end()) {
				param_index ++;
			}

			pb::ttemplate_param* param = temp_template_.add_params();
			param->set_index(param_index);
			param->set_field_id(field_id);

			ss << "%" << std::setfill('0') << std::setw(2) << param_index;
			used_params.insert(std::make_pair(field_id, param_index));

			param_index ++;

		} else {
			if (it != parsed_items.end()) {
				ss << label.substr(before_it->first, it->first - before_it->first);
			} else {
				ss << label.substr(before_it->first);
			}
		}

		if (it == parsed_items.end()) {
			break;
		}
	}
	VALIDATE(used_params.size() == temp_template_.params_size(), null_str);

	temp_template_.set_message(ss.str());

	window.set_retval(twindow::OK);
}

bool tedit_template::can_save()
{
	ttext_box* text_box = find_widget<ttext_box>(window_, "name", false, true);
	std::string label = text_box->label();
	if (label.empty()) {
		return false;
	}

	tscroll_text_box* scroll_text_box = find_widget<tscroll_text_box>(window_, "message", false, true);
	label = scroll_text_box->label();
	if (label.empty()) {
		return false;
	}

	return true;
}

void tedit_template::did_field_changed()
{
	tbutton* ok = find_widget<tbutton>(window_, "save", false, true);
	ok->set_active(can_save());
}

void tedit_template::did_message_extra_menu(ttext_box& widget, std::vector<tfloat_widget*>& menu)
{
	twindow* window = get_window();

	const std::string field_id = "flt_field";
	menu.push_back(window->find_float_widget(field_id));

}

} // namespace gui2

