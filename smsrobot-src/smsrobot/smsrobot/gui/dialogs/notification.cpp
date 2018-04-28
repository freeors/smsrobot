#define GETTEXT_DOMAIN "smsrobot-lib"

#include "gui/dialogs/notification.hpp"

#include "gui/widgets/label.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/window.hpp"
#include "gui/dialogs/message.hpp"
#include "gettext.hpp"

#include "game_config.hpp"
#include "network.hpp"

#include <boost/bind.hpp>

namespace gui2 {

REGISTER_DIALOG(smsrobot, notification)

tnotification::tnotification(const tsend_info& send_info, pb::tcontacts& contacts, pb::ttemplates& templates, const std::string& layer)
	: send_info_(send_info)
	, contacts_(contacts)
	, templates_(templates)
	, layer_(layer)
{
}

void tnotification::pre_show()
{
	window_->set_label("misc/white-background.png");

	const std::string do_png = "misc/swipe-left.png";
	const std::string normal_png = "misc/message-color96.png";
	tlistbox* listbox = find_widget<tlistbox>(window_, "notifications", false, true);
	listbox->enable_select(false);
	std::map<std::string, std::string> data;
	if (current_user.valid()) {
		data["portrait"] = do_png;
		int cookie = do_none;
		std::string label_str;
		if (current_user.contacts_ts > contacts_.timestamp()) {
			label_str = _("contacts.pb on server is newer than local, do you want download?");
			cookie = download_contacts;
		} else if (current_user.contacts_ts < contacts_.timestamp()) {
			label_str = _("contacts.pb on local is newer than server, do you want upload?");
			cookie = upload_contacts;
		}
		if (cookie) {
			data["label"] = label_str;
			ttoggle_panel& row = listbox->insert_row(data);
			row.set_cookie(cookie);
		}
		cookie = 0;
		if (current_user.templates_ts > templates_.timestamp()) {
			label_str = _("templates.pb on server is newer than local, do you want download?");
			cookie = download_templates;
		} else if (current_user.templates_ts < templates_.timestamp()) {
			label_str = _("templates.pb on local is newer than server, do you want upload?");
			cookie = upload_templates;
		}
		if (cookie) {
			data["label"] = label_str;
			ttoggle_panel& row = listbox->insert_row(data);
			row.set_cookie(cookie);
		}

	} else {
		// data["portrait"] = normal_png;
		data["portrait"] = do_png;
		data["label"] = _("You are not logged in. Once logged in, can synchronize contacts and template with server.");
		ttoggle_panel& row = listbox->insert_row(data);
		int cookie = login;
		if (cookie) {
			row.set_cookie(login);
		}
	}
	listbox->set_did_can_drag(boost::bind(&tnotification::did_row_can_drag, this, _1, _2));

	tbutton* button = dynamic_cast<tbutton*>(listbox->left_drag_grid()->find("handle", true));
	button->set_icon("misc/red-background.png");
	connect_signal_mouse_left_click(
		*button
		, boost::bind(
			&tnotification::handle
			, this
			, boost::ref(*window_)
			, boost::ref(*listbox)));

	button = find_widget<tbutton>(window_, "save", false, true);
	button->set_icon("misc/back.png");
	button->set_label(layer_);
	connect_signal_mouse_left_click(
			  *button
			, boost::bind(
				&tnotification::save
				, this, boost::ref(*window_)));
}

void tnotification::post_show()
{
}

void tnotification::save(twindow& window)
{
	window.set_retval(twindow::OK);
}

bool tnotification::did_row_can_drag(tlistbox& listbox, ttoggle_panel& row) const
{
	const int cookie = row.cookie();
	tbutton* button = dynamic_cast<tbutton*>(listbox.left_drag_grid()->find("handle", true));

	if (cookie == login) {
		button->set_label(_("Jump login"));
		return true;

	} else if (cookie == upload_contacts || cookie == upload_templates) {
		button->set_label(_("Upload"));
		return true;

	} else if (cookie == download_contacts || cookie == download_templates) {
		button->set_label(_("Download"));
		return true;
	}

	return false;
}

void tnotification::handle(twindow& window, tlistbox& listbox)
{
	const int drag_at = listbox.drag_at();
	ttoggle_panel& row = listbox.row_panel(drag_at);

	const int cookie = row.cookie();
	if (cookie == login) {
		window.set_retval(LOGIN);
		return;
	}

	std::stringstream err;
	if (!current_user.valid()) {
		err << _("You are not logged in");
		gui2::show_message(null_str, err.str());
		return;
	}

	if (cookie == upload_contacts || cookie == upload_templates) {
		http_agent agent;
		std::map<int, int64_t> files;
		if (cookie == upload_contacts) {
			files.insert(std::make_pair(CONTACTS_PB, contacts_.timestamp()));
		} else {
			files.insert(std::make_pair(TEMPLATES_PB, templates_.timestamp()));
		}
		bool ok = agent.upload_pb(current_user.uid, current_user.sessionid, files);
		if (ok) {
			if (cookie == upload_contacts) {
				current_user.contacts_ts = contacts_.timestamp();
			} else {
				current_user.templates_ts = templates_.timestamp();
			}
			listbox.erase_row(drag_at);

		} else if (!current_user.valid()) {
			window.set_retval(LOGIN);
		}

	} else {
		VALIDATE(cookie == download_contacts || cookie == download_templates, null_str);
		if (!send_info_.can_edit_contacts_or_templates()) {
			err << _("Current is using send queue, can not download.");
			gui2::show_message(null_str, err.str());
			return;
		}

		http_agent agent;
		std::set<int> files;
		if (cookie == download_contacts) {
			files.insert(CONTACTS_PB);
		} else {
			files.insert(TEMPLATES_PB);
		}
		bool ok = agent.download_pb(current_user.uid, current_user.sessionid, files);
		if (ok) {
			if (cookie == download_contacts) {
				load_pb(CONTACTS_PB, contacts_);
			} else {
				load_pb(TEMPLATES_PB, templates_);
			}
			listbox.erase_row(drag_at);

		} else if (!current_user.valid()) {
			window.set_retval(LOGIN);
		}
	}
}

} // namespace gui2

