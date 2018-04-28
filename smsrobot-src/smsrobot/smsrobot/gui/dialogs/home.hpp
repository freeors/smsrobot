#ifndef GUI_DIALOGS_HOME_HPP_INCLUDED
#define GUI_DIALOGS_HOME_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "sms_smart_android.h"
#include "thread.hpp"
#include "contacts.pb.h"
#include "templates.pb.h"

namespace gui2 {

class treport;
class ttoggle_button;
class ttoggle_panel;
class tlistbox;
class tstack;

class thome: public tdialog
{
public:
	enum tresult {EDIT_GROUP = 1, EDIT_TEMPLATE, EDIT_FIELD, NOTIFICATION, LOGIN, CHANGE_LANGUAGE};
	enum {SMS_LAYER, GROUP_LAYER, TEMPLATE_LAYER, FIELD_LAYER, MORE_LAYER};

	explicit thome(const std::vector<tsim_info>& sim_infos, tsend_info& send_info, tsms_smart& sms_smart, std::vector<tsend_record>& send_records, pb::tcontacts& contacts, pb::ttemplates& templates, const tparameters& parameters, const boost::function<void (const bool)>& did_run_task, int start_layer);
	void did_refresh_queue_list(bool erase);
	void did_refresh_task_buttons();
	void did_sms_sent(const int at);
	int context_result() const { return context_result_; }

	void refresh_notification_widget(int layer);

private:
	/** Inherited from tdialog. */
	void pre_show() override;

	/** Inherited from tdialog. */
	void post_show() override;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	bool group_and_template_ready() const;
	bool has_notification() const;
	void handle_notification(twindow& window, int layer);
	bool field_in_first_template(const std::string& id) const;
	void did_body_changed(treport& report, ttoggle_button& widget);

	// sms
	void pre_sms(twindow& window);
	void reload_sms_layer();
	void select_sim_strategy(twindow& window);
	void select_interval(twindow& window);
	void reset_task(twindow& window);
	void run_task(twindow& window);

	// group
	void pre_group(twindow& window);
	void fill_groups_listbox(tlistbox& listbox);
	void post_group_row_ui(ttoggle_panel& row);
	void require_send_group(tlistbox& listbox);
	void clone_group(tlistbox& listbox);
	void erase_group(tlistbox& listbox);
	void new_group(twindow& window);
	bool did_group_can_drag(tlistbox& listbox, ttoggle_panel& row);
	void did_edit_group(ttoggle_panel& row);

	// template
	void pre_template(twindow& window);
	void fill_templates_listbox(tlistbox& listbox);
	void post_template_row_ui(ttoggle_panel& row);
	void new_template(twindow& window);
	void require_send_template(tlistbox& listbox);
	void clone_template(tlistbox& listbox);
	void erase_template(twindow& window, tlistbox& listbox);
	bool did_template_can_drag(tlistbox& listbox, ttoggle_panel& row);
	void did_edit_template(ttoggle_panel& row);

	// field
	void pre_field(twindow& window);
	void fill_fields_listbox(tlistbox& listbox);
	void post_field_row_ui(ttoggle_panel& row, bool fixed, const std::string& field_id);
	void new_field(twindow& window);
	void erase_field(twindow& window, tlistbox& listbox);
	void did_edit_field(ttoggle_panel& row);
	bool did_fields_can_drag(ttoggle_panel& row) const;

	// more
	void pre_more(twindow& window);
	void update_relative_login(twindow& window);

	void set_bbs_url(twindow& window);
	void logout(twindow& window);
	void set_retval(twindow& window, int retval);

private:
	tstack* body_;
	const std::vector<tsim_info>& sim_infos_;
	tsend_info& send_info_;
	tsms_smart& sms_smart_;
	std::vector<tsend_record>& send_records_;
	const tparameters& parameters_;
	boost::function<void (const bool)> did_run_task_;
	int start_layer_;
	std::map<int, std::string> intervals_;

	threading::mutex history_mutex_;
	pb::tcontacts& contacts_;
	pb::ttemplates& templates_;

	bool reload_sms_layer_;
	bool reload_template_;
	int context_result_;
};

} // namespace gui2

#endif

