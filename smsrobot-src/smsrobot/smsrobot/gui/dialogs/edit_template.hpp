#ifndef GUI_DIALOGS_EDIT_TEMPLATE_HPP_INCLUDED
#define GUI_DIALOGS_EDIT_TEMPLATE_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "gui/widgets/control.hpp"
#include "contacts.pb.h"
#include "templates.pb.h"
#include "game_config.hpp"

namespace gui2 {

class ttext_box;

class tedit_template: public tdialog
{
public:
	enum tresult {OCR = 1};
	explicit tedit_template(const pb::tcontacts& contacts, const pb::ttemplates& templates, pb::ttemplate& template2, bool edit, const tparameters& parameters, bool read_only);

	const pb::ttemplate& get_template() const { return temp_template_; }

private:
	/** Inherited from tdialog. */
	void pre_show() override;

	/** Inherited from tdialog. */
	void post_show() override;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	void float_widget_click_field(twindow& window);
	std::set<int> extract_parameters() const;
	void did_message_extra_menu(ttext_box& widget, std::vector<tfloat_widget*>& menu);

	void ocr(twindow& window);
	void save(twindow& window);
	bool can_save();

	pb::ttemplate_param* find_param(const int index);
	void did_field_changed();

private:
	bool edit_;
	const pb::tcontacts& contacts_; 
	pb::ttemplate& temp_template_;
	const tparameters& parameters_;
	bool read_only_;
};

} // namespace gui2

#endif

