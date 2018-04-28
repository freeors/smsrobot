#ifndef GUI_DIALOGS_NOTIFICATION_HPP_INCLUDED
#define GUI_DIALOGS_NOTIFICATION_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "game_config.hpp"
#include "contacts.pb.h"
#include "templates.pb.h"

namespace gui2 {

class tlistbox;
class ttoggle_panel;

class tnotification: public tdialog
{
public:
	enum tresult {LOGIN = 1};

	enum {do_none, upload_contacts, upload_templates, download_contacts, download_templates, login};
	explicit tnotification(const tsend_info& send_info, pb::tcontacts& contacts, pb::ttemplates& templates, const std::string& layer);

private:
	/** Inherited from tdialog. */
	void pre_show() override;

	/** Inherited from tdialog. */
	void post_show() override;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	bool did_row_can_drag(tlistbox& listbox, ttoggle_panel& row) const;
	void save(twindow& window);
	void handle(twindow& window, tlistbox& listbox);

private:
	const tsend_info& send_info_;
	pb::tcontacts& contacts_;
	pb::ttemplates& templates_;
	const std::string layer_;
};

} // namespace gui2

#endif

