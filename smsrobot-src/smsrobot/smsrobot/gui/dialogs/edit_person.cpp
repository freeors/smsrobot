#define GETTEXT_DOMAIN "smsrobot-lib"

#include "gui/dialogs/edit_person.hpp"

#include "gui/widgets/label.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/toggle_panel.hpp"

#include "game_config.hpp"
#include "gettext.hpp"
#include "filesystem.hpp"

#include <boost/bind.hpp>

namespace gui2 {

REGISTER_DIALOG(smsrobot, edit_person)

tedit_person::tedit_person(const pb::tcontacts& contacts, pb::tperson& person, bool edit, const std::set<std::string>& bh_fields, bool read_only)
	: contacts_(contacts)
	, temp_person_(person)
	, edit_(edit)
	, bh_fields_(bh_fields)
	, read_only_(read_only)
{
	if (!edit) {
		// temp_person_.set_name(_("Untitled contacts"));
	}
}

void tedit_person::pre_show()
{
	window_->set_label("misc/white-background.png");

	std::stringstream ss;
	ss.str("");
	if (edit_) {
		if (!read_only_) {
			ss << _("Edit person");
		} else {
			ss << _("Browse person") << "(" << _("Read only") << ")";
		}
	} else {
		ss << _("Insert person");
	}
	find_widget<tlabel>(window_, "title", false, true)->set_label(ss.str());

	tpanel* name_panel = find_widget<tpanel>(window_, "name_panel", false, true);
	name_panel->set_border(null_str);

	ttext_box* text_box = find_widget<ttext_box>(window_, "name", false, true);
	text_box->set_border(null_str);
	text_box->set_placeholder(_("person^Name"));
	text_box->set_label(temp_person_.name());
	text_box->set_did_text_changed(boost::bind(&tedit_person::did_field_changed, this));
	if (read_only_) {
		text_box->set_active(false);
	}

	text_box = find_widget<ttext_box>(window_, "money", false, true);
	text_box->set_border(null_str);
	text_box->set_placeholder(_("money^Sum"));
	text_box->set_label(temp_person_.money());
	text_box->set_did_text_changed(boost::bind(&tedit_person::did_field_changed, this));
	if (read_only_) {
		text_box->set_active(false);
	}

	// mobiles
	tlistbox* mobiles = find_widget<tlistbox>(window_, "mobiles", false, true);
	mobiles->enable_select(false);
	mobiles->set_did_longpress_menu(boost::bind(&tedit_person::did_longpress_menu, this, _1, _2, _3));
	fill_mobiles_listbox(*mobiles);
	if (!read_only_) {
		insert_mobile(*window_);
	}

	// bh fields
	tlistbox* bh_fields = find_widget<tlistbox>(window_, "bh_fields", false, true);
	bh_fields->enable_select(false);
	std::map<std::string, std::string> data;

	for (int at = 0; at < contacts_.bh_fields_size(); at ++) {
		const pb::tbh_field& bh_field = contacts_.bh_fields(at);
		if (bh_fields_.find(bh_field.id()) == bh_fields_.end()) {
			continue;
		}

		pb::tperson_bh_field* proto_bh_field = find_bh_field(bh_field.id());
		data["name"] = bh_field.name();
		data["value"] = proto_bh_field? proto_bh_field->value(): null_str;
		
		ttoggle_panel& row = bh_fields->insert_row(data);
		row.set_cookie(at);
		row.set_canvas_highlight(false, true);
		ttext_box* value = dynamic_cast<ttext_box*>(row.find("value", false));
		value->set_border(null_str);
		value->set_placeholder(bh_field.name());
		if (read_only_) {
			value->set_active(false);
		}
	}

	tbutton* button = find_widget<tbutton>(window_, "save", false, true);
	if (!read_only_) {
		connect_signal_mouse_left_click(
			  *button
			, boost::bind(
				&tedit_person::save
				, this, boost::ref(*window_)));
	} else {
		button->set_visible(twidget::HIDDEN);
	}

	// set float button in extra menu
	const std::string erase_id = "flt_erase";
	tfloat_widget* float_widget = window_->find_float_widget(erase_id);
	float_widget->widget->set_label(_("Erase row"));
	float_widget->widget->set_border("float_widget");
	connect_signal_mouse_left_click(
			*float_widget->widget
		, boost::bind(
				&tedit_person::float_widget_click_erase
			, this
			, boost::ref(*mobiles)));

	did_field_changed();
}

void tedit_person::post_show()
{
}

std::string generate_mobile_tag(int at)
{
	return std::string(_("Mobile")) + str_cast(at + 1);
}

void tedit_person::fill_mobiles_listbox(tlistbox& listbox)
{
	listbox.clear();
	std::map<std::string, std::string> data;
	const int proto_mobiles = temp_person_.mobiles_size();
	for (int at = 0; at < proto_mobiles; at ++) {
		const pb::tperson_mobile& proto_mobile = temp_person_.mobiles(at);
		data["tag"] = generate_mobile_tag(at);
		data["number"] = proto_mobile.number();
		
		ttoggle_panel& row = listbox.insert_row(data);
		row.set_canvas_highlight(false, true);
		ttext_box* number = dynamic_cast<ttext_box*>(row.find("number", false));
		number->set_border(null_str);
		number->set_placeholder(_("mobile^Number"));
		number->set_did_text_changed(boost::bind(&tedit_person::did_number_changed, this, boost::ref(listbox), boost::ref(row), _1));
		if (read_only_) {
			number->set_active(false);
		}

		tbutton* erase = dynamic_cast<tbutton*>(row.find("erase", false));
		erase->set_cookie(at);
		if (read_only_) {
			erase->set_active(false);
		}
		connect_signal_mouse_left_click(
			  *erase
			, boost::bind(
				&tedit_person::erase_mobile
				, this
				, boost::ref(listbox)
				, boost::ref(*erase)));
	}
}

bool tedit_person::mobile_number_ok(const std::string& number) const
{
	const int mobile_number_size = 11;
	if (number.size() != mobile_number_size) {
		return false;
	}
	if (!utils::isinteger(number)) {
		return false;
	}
	return true;
}

bool tedit_person::can_save()
{
	// name
	ttext_box* text_box = find_widget<ttext_box>(window_, "name", false, true);
	temp_person_.set_name(text_box->label());
	if (temp_person_.name().empty()) {
		return false;
	}

	// money
	text_box = find_widget<ttext_box>(window_, "money", false, true);
	temp_person_.set_money(text_box->label());	

	// mobiles
	tlistbox* mobiles = find_widget<tlistbox>(window_, "mobiles", false, true);
	int rows = mobiles->rows();
	if (!rows) {
		return true;
	}
	for (int at = 0; at < rows; at ++) {
		ttoggle_panel& row = mobiles->row_panel(at);
		ttext_box* number = dynamic_cast<ttext_box*>(row.find("number", false));

		std::string label = number->label();
		if (!label.empty() && !mobile_number_ok(label)) {
			return false;
		}
	}

	return true;
}

void tedit_person::save(twindow& window)
{
	temp_person_.clear_mobiles();

	tlistbox* mobiles = find_widget<tlistbox>(window_, "mobiles", false, true);
	int rows = mobiles->rows();
	for (int at = 0; at < rows; at ++) {
		ttoggle_panel& row = mobiles->row_panel(at);
		ttext_box* number = dynamic_cast<ttext_box*>(row.find("number", false));
		const std::string& label = number->label();
		if (!label.empty()) {
			pb::tperson_mobile* mobile = temp_person_.add_mobiles();
			mobile->set_number(number->label());
		}
	}

	temp_person_.clear_bh_fields();
	tlistbox* bh_fields = find_widget<tlistbox>(window_, "bh_fields", false, true);
	rows = bh_fields->rows();
	for (int at = 0; at < rows; at ++) {
		ttoggle_panel& row = bh_fields->row_panel(at);

		const pb::tbh_field& bh_field = contacts_.bh_fields(row.cookie());
		ttext_box* value = dynamic_cast<ttext_box*>(row.find("value", false));
		pb::tperson_bh_field* proto_bh_field = temp_person_.add_bh_fields();
		proto_bh_field->set_field_id(bh_field.id());
		proto_bh_field->set_value(value->label());
	}

	window.set_retval(twindow::OK);
}

void tedit_person::insert_mobile(twindow& window)
{
	tlistbox* mobiles = find_widget<tlistbox>(window_, "mobiles", false, true);
	const int rows = mobiles->rows();

	std::map<std::string, std::string> data;
	data["tag"] = std::string(_("Mobile")) + str_cast(rows + 1);
	data["number"] = null_str;
		
	ttoggle_panel& row = mobiles->insert_row(data);
	row.set_canvas_highlight(false, true);
	ttext_box* number = dynamic_cast<ttext_box*>(row.find("number", false));
	number->set_border(null_str);
	number->set_placeholder(_("mobile^Number"));
	number->set_did_text_changed(boost::bind(&tedit_person::did_number_changed, this, boost::ref(*mobiles), boost::ref(row), _1));

	tbutton* erase = dynamic_cast<tbutton*>(row.find("erase", false));
	erase->set_cookie(rows);
	connect_signal_mouse_left_click(
			  *erase
			, boost::bind(
				&tedit_person::erase_mobile
				, this
				, boost::ref(*mobiles)
				, boost::ref(*erase)));

	did_field_changed();
}

void tedit_person::erase_mobile(tlistbox& listbox, tcontrol& widget)
{
	const int erase_at = widget.cookie();
	listbox.erase_row(erase_at);

	const int rows = listbox.rows();
	for (int at = erase_at; at < rows; at ++) {
		ttoggle_panel& row = listbox.row_panel(at);
		tlabel* tag = dynamic_cast<tlabel*>(row.find("tag", false));
		tag->set_label(generate_mobile_tag(at));

		tbutton* erase = dynamic_cast<tbutton*>(row.find("erase", false));
		erase->set_cookie(at);
	}

	did_field_changed();
}

void tedit_person::did_number_extra_menu(ttext_box& widget, std::vector<tfloat_widget*>& menu)
{
	twindow* window = get_window();

	const std::string erase_id = "flt_erase";
	menu.push_back(window->find_float_widget(erase_id));

}

void tedit_person::did_longpress_menu(tlistbox& list, ttoggle_panel& row, std::vector<tfloat_widget*>& menu)
{
	if (read_only_) {
		return;
	}

	twindow* window = get_window();

	const std::string erase_id = "flt_erase";
	menu.push_back(window->find_float_widget(erase_id));
}

void tedit_person::did_number_changed(tlistbox& listbox, ttoggle_panel& row, ttext_box& widget)
{
	const int mobile_number_size = 11;
	if (widget.label().size() == mobile_number_size && row.at() == listbox.rows() - 1) {
		insert_mobile(*window_);
	}
	did_field_changed();
}

void tedit_person::did_field_changed()
{
	tbutton* ok = find_widget<tbutton>(window_, "save", false, true);
	ok->set_active(can_save());
}

pb::tperson_bh_field* tedit_person::find_bh_field(const std::string& id)
{
	int fields = temp_person_.bh_fields_size();
	for (int at = 0; at < fields; at ++) {
		if (temp_person_.bh_fields(at).field_id() == id) {
			return temp_person_.mutable_bh_fields(at);
		}
	}
	return nullptr;
}

void tedit_person::float_widget_click_erase(tlistbox& mobiles)
{
	const int longpress_at = mobiles.last_longpress_at();

	mobiles.erase_row(longpress_at);
	const int rows = mobiles.rows();
	for (int at = longpress_at; at < rows; at ++) {
		ttoggle_panel& row = mobiles.row_panel(at);
		tlabel* tag = dynamic_cast<tlabel*>(row.find("tag", false));
		tag->set_label(generate_mobile_tag(at));
	}

	did_field_changed();
}

} // namespace gui2

