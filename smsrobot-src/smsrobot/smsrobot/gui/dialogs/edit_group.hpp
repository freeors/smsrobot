#ifndef GUI_DIALOGS_EDIT_GROUP_HPP_INCLUDED
#define GUI_DIALOGS_EDIT_GROUP_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "contacts.pb.h"
#include "game_config.hpp"

namespace gui2 {

class ttoggle_panel;
class tlistbox;

class tedit_group: public tdialog
{
public:
	enum tresult {EDIT_PERSON = 1, OCR};
	explicit tedit_group(const pb::tcontacts& contacts, pb::tgroup& group, bool edit, const tparameters& parameters, bool read_only);

	int context_result() const { return context_result_; }

private:
	/** Inherited from tdialog. */
	void pre_show() override;

	/** Inherited from tdialog. */
	void post_show() override;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	void new_private_person(twindow& window);
	bool did_person_can_drag(ttoggle_panel& row);
	void did_edit_private_person(ttoggle_panel& row);

	void fill_persons_listbox(tlistbox& listbox);
	std::string generate_fields_description();

	void erase_person(tlistbox& listbox);
	void edit_field(twindow& window);
	bool can_save();
	void did_field_changed();
	void save(twindow& window);
	void ocr(twindow& window);

private:
	const bool edit_;
	const pb::tcontacts& contacts_;
	const tparameters& parameters_;
	bool read_only_;
	pb::tgroup& temp_group_;
	int context_result_;
};

} // namespace gui2

#endif

