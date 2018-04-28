#define GETTEXT_DOMAIN "smsrobot-lib"

#include "gui/dialogs/edit_field.hpp"

#include "gui/widgets/label.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"
#include "gettext.hpp"

#include <boost/bind.hpp>

namespace gui2 {

REGISTER_DIALOG(smsrobot, edit_field)

tedit_field::tedit_field(const pb::tcontacts& contacts, pb::tbh_field& bh_field, bool edit, const tparameters& parameters)
	: contacts_(contacts)
	, temp_bh_field_(bh_field)
	, edit_(edit)
	, parameters_(parameters)
{
}

#define MIN_ID_CHARS		2
#define MAX_ID_CHARS		32
#define MIN_NAME_CHARS		1
#define MAX_NAME_CHARS		32

void tedit_field::pre_show()
{
	window_->set_label("misc/white-background.png");

	ttext_box* text_box = find_widget<ttext_box>(window_, "id", false, true);
	text_box->set_border(null_str);
	text_box->set_maximum_chars(MAX_ID_CHARS);
	text_box->set_placeholder(_("ID"));
	text_box->set_label(temp_bh_field_.id());
	if (edit_) {
		VALIDATE(!temp_bh_field_.id().empty(), null_str);
		text_box->set_active(false);
	} else {
		text_box->set_did_text_changed(boost::bind(&tedit_field::did_field_changed, this));
		window_->keyboard_capture(text_box);
	}

	text_box = find_widget<ttext_box>(window_, "name", false, true);
	text_box->set_border(null_str);
	text_box->set_maximum_chars(MAX_ID_CHARS);
	text_box->set_placeholder(_("Name"));
	text_box->set_label(temp_bh_field_.name());
	text_box->set_did_text_changed(boost::bind(&tedit_field::did_field_changed, this));
	if (edit_) {
		window_->keyboard_capture(text_box);
	}

	const int proto_bh_fields = contacts_.bh_fields_size();
	for (int at = 0; at < proto_bh_fields; at ++) {
		const pb::tbh_field& proto_bh_field = contacts_.bh_fields(at);
		used_ids_.insert(proto_bh_field.id());
	}

	did_field_changed();

	connect_signal_mouse_left_click(
			  find_widget<tbutton>(window_, "save", false)
			, boost::bind(
				&tedit_field::save
				, this, boost::ref(*window_)));
}

void tedit_field::post_show()
{
}

void tedit_field::save(twindow& window)
{
	ttext_box* text_box = find_widget<ttext_box>(window_, "id", false, true);
	temp_bh_field_.set_id(text_box->label());

	text_box = find_widget<ttext_box>(window_, "name", false, true);
	temp_bh_field_.set_name(text_box->label());

	window.set_retval(twindow::OK);
}

bool tedit_field::can_save()
{
	ttext_box* text_box = find_widget<ttext_box>(window_, "id", false, true);
	std::string label = text_box->label();
	if (label.empty()) {
		return false;
	}
	if (!utils::isvalid_id(label, true, MIN_ID_CHARS, MAX_ID_CHARS)) {
		return false;
	}
	if (!edit_) {
		if (parameters_.is_existed(label)) {
			return false;
		}
		if (used_ids_.count(label)) {
			return false;
		}
	}

	text_box = find_widget<ttext_box>(window_, "name", false, true);
	label = text_box->label();
	if (label.empty()) {
		return false;
	}
	int chars = utils::utf8str_len(label);
	if (chars < MIN_NAME_CHARS) {
		return false;
	}

	return true;
}

void tedit_field::did_field_changed()
{
	tbutton* ok = find_widget<tbutton>(window_, "save", false, true);
	ok->set_active(can_save());
}

} // namespace gui2

