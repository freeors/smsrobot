#define GETTEXT_DOMAIN "smsrobot-lib"

#include "gui/dialogs/edit_group.hpp"

#include "gui/widgets/label.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/dialogs/multiple_selector.hpp"
#include "gui/dialogs/message2.hpp"
#include "gettext.hpp"
#include "formula_string_utils.hpp"
#include "font.hpp"

#include <boost/bind.hpp>

namespace gui2 {

REGISTER_DIALOG(smsrobot, edit_group)

tedit_group::tedit_group(const pb::tcontacts& contacts, pb::tgroup& group, bool edit, const tparameters& parameters, bool read_only)
	: contacts_(contacts)
	, parameters_(parameters)
	, read_only_(read_only)
	, temp_group_(group)
	, edit_(edit)
{
	if (!edit) {
		VALIDATE(!read_only, null_str);
		temp_group_.set_name(_("Untitled group"));
	}
}

void tedit_group::pre_show()
{
	window_->set_label("misc/white-background.png");

	std::stringstream ss;
	ss.str("");
	if (edit_) {
		if (!read_only_) {
			ss << _("Edit group");
		} else {
			ss << _("Browse group") << "(" << _("Read only") << ")";
		}
	} else {
		ss << _("Insert group");
	}
	find_widget<tlabel>(window_, "title", false, true)->set_label(ss.str());

	ttext_box* text_box = find_widget<ttext_box>(window_, "name", false, true);
	text_box->set_border(null_str);
	text_box->set_placeholder(_("Name"));
	text_box->set_label(temp_group_.name());
	text_box->set_did_text_changed(boost::bind(&tedit_group::did_field_changed, this));
	if (read_only_) {
		text_box->set_active(false);
	}

	//
	// private persons
	//
	tlistbox* private_persons = find_widget<tlistbox>(window_, "private_persons", false, true);
	private_persons->enable_select(false);
	fill_persons_listbox(*private_persons);
	private_persons->set_did_can_drag(boost::bind(&tedit_group::did_person_can_drag, this,_2));
#ifdef _WIN32
	private_persons->set_did_row_double_click(boost::bind(&tedit_group::did_edit_private_person, this, _2));
#else
	private_persons->set_did_row_changed(boost::bind(&tedit_group::did_edit_private_person, this, _2));
#endif

	tbutton* button = find_widget<tbutton>(window_, "edit_field", false, true);
	connect_signal_mouse_left_click(
			  *button
			, boost::bind(
				&tedit_group::edit_field
				, this, boost::ref(*window_)));
	button->set_label(generate_fields_description());

	button = find_widget<tbutton>(window_, "insert_private", false, true);
	if (!read_only_) {
		connect_signal_mouse_left_click(
			  *button
			, boost::bind(
				&tedit_group::new_private_person
				, this, boost::ref(*window_)));
	} else {
		button->set_visible(twidget::HIDDEN);
	}

	button = dynamic_cast<tbutton*>(private_persons->left_drag_grid()->find("erase", true));
	button->set_icon("misc/red-background.png");
	connect_signal_mouse_left_click(
		*button
		, boost::bind(
			&tedit_group::erase_person
			, this
			, boost::ref(*private_persons)));

	tbutton& save = find_widget<tbutton>(window_, "save", false);
	save.set_icon("misc/back.png");
	connect_signal_mouse_left_click(
			  save
			, boost::bind(
				&tedit_group::save
				, this, boost::ref(*window_)));

	connect_signal_mouse_left_click(
			find_widget<tbutton>(window_, "ocr", false)
		, boost::bind(
				&tedit_group::ocr
			, this
			, boost::ref(*window_)));

	did_field_changed();
}

std::string tedit_group::generate_fields_description()
{
	std::stringstream fields_ss;
	const int proto_fields = temp_group_.fields_size();
	for (int at = 0; at < proto_fields; at ++) {
		if (!fields_ss.str().empty()) {
			fields_ss << "   ";
		}
		const std::string& field_id = temp_group_.fields(at);
		const pb::tbh_field* bh_field = find_bh_field_from_contacts(contacts_, field_id);
		if (bh_field) {
			const std::string& name = bh_field->name();
			fields_ss << generate_highlight_template_field(field_id, name);
		}
	}
	if (fields_ss.str().empty()) {
		fields_ss << generate_highlight_template_field("tip", _("No extra field"));
	}
	return fields_ss.str();
}

void tedit_group::post_show()
{
}

void tedit_group::fill_persons_listbox(tlistbox& listbox)
{
	std::stringstream ss;
	utils::string_map symbols;
	std::map<std::string, std::string> data;

	tlistbox* private_persons = find_widget<tlistbox>(window_, "private_persons", false, true);
	listbox.clear();
	private_persons->enable_select(false);
	const int proto_private_persons = temp_group_.persons_size();
	for (int at = 0; at < proto_private_persons; at ++) {
		const pb::tperson& proto_person = temp_group_.persons(at);

		data["name"] = proto_person.name();

		symbols["mobiles"] = str_cast(proto_person.mobiles_size());
		symbols["money"] = proto_person.money();
		ss.str("");
		ss << vgettext2("$mobiles mobile number, $money", symbols);
		data["mobiles"] = ss.str();

		ttoggle_panel& row = private_persons->insert_row(data);
		tcontrol& control = *dynamic_cast<tcontrol*>(row.find("portrait", false));
		std::vector<tformula_blit> blits;
		blits.push_back(tformula_blit("misc/person.png", null_str, null_str, "(width)", "(height)"));
		surface surf = font::get_rendered_text(str_cast(at + 1), 0, font::SIZE_DEFAULT, font::BLACK_COLOR);
		blits.push_back(tformula_blit(surf, 0, 0, surf->w, surf->h));
		control.set_blits(blits);
	}
}

void tedit_group::erase_person(tlistbox& listbox)
{
	const int drag_at = listbox.drag_at();

	std::stringstream ss;
	utils::string_map symbols;
	symbols["person"] = temp_group_.persons(drag_at).name();
	ss << vgettext2("Do you want to delete person: $person|?", symbols);

	const int res = gui2::show_message2(ss.str(), _("Delete"));
	if (res == gui2::twindow::CANCEL) {
		return;
	}

	// post handle
	if (drag_at != temp_group_.persons_size() - 1) {
		int at = drag_at;
		for (int times = 0; times < temp_group_.persons_size() - 1 - drag_at; times ++, at ++) {
			temp_group_.mutable_persons()->SwapElements(at, at + 1);
		}
	}
	temp_group_.mutable_persons()->RemoveLast();

	fill_persons_listbox(listbox);
}

void tedit_group::edit_field(twindow& window)
{
	if (read_only_) {
		return;
	}

	std::vector<std::string> items;
	int at = 0;
	int value = nposm;
	std::stringstream ss;
	int bh_fields = contacts_.bh_fields_size();
	if (bh_fields == 0) {
		return;
	}

	std::vector<std::string> ids;

	for (int at = 0; at < bh_fields; at ++) {
		const pb::tbh_field& bh_field = contacts_.bh_fields(at);
		ss.str("");
		ss << bh_field.name() << "(" << ht::generate_format(bh_field.id(), 0xff0000ff) << ")";
		items.push_back(ss.str());
		ids.push_back(bh_field.id());
	}

	std::set<int> initial_selected;
	const int proto_fields = temp_group_.fields_size();
	for (int at = 0; at < proto_fields; at ++) {
		const std::string& id = temp_group_.fields(at);
		std::vector<std::string>::iterator it = std::find(ids.begin(), ids.end(), id);
		if (it != ids.end()) {
			initial_selected.insert(std::distance(ids.begin(), it));
		}
	}

	gui2::tmultiple_selector dlg(_("Select field"), items, initial_selected);
	dlg.show();
	if (dlg.get_retval() != twindow::OK) {
		return;
	}

	temp_group_.clear_fields();
	const std::set<int>& selected = dlg.selected();
	for (std::set<int>::const_iterator it = selected.begin(); it != selected.end(); ++ it) {
		int at = *it;
		const pb::tbh_field& bh_field = contacts_.bh_fields(at);
		std::string id = bh_field.id();
		temp_group_.add_fields(id);
	}
	tbutton* button = find_widget<tbutton>(window_, "edit_field", false, true);
	button->set_label(generate_fields_description());
}

void tedit_group::new_private_person(twindow& window)
{
	context_result_ = nposm;
	window.set_retval(EDIT_PERSON);
}

bool tedit_group::did_person_can_drag(ttoggle_panel& row)
{
	return !read_only_;
}

void tedit_group::did_edit_private_person(ttoggle_panel& row)
{
	context_result_ = row.at();
	window_->set_retval(EDIT_PERSON);
}

void tedit_group::save(twindow& window)
{
	window.set_retval(twindow::OK);
}

void tedit_group::ocr(twindow& window)
{
	window.set_retval(OCR);
}

bool tedit_group::can_save()
{
	ttext_box* text_box = find_widget<ttext_box>(window_, "name", false, true);
	temp_group_.set_name(text_box->label());
	if (temp_group_.name().empty()) {
		return false;
	}
/*
	tlistbox* private_persons = find_widget<tlistbox>(window_, "private_persons", false, true);
	if (private_persons->rows() == 0) {
		return false;
	}
*/
	return true;
}

void tedit_group::did_field_changed()
{
	tbutton* ok = find_widget<tbutton>(window_, "save", false, true);
	ok->set_active(can_save());
}

} // namespace gui2

