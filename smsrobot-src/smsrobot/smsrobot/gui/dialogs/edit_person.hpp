#ifndef GUI_DIALOGS_EDIT_PERSON_HPP_INCLUDED
#define GUI_DIALOGS_EDIT_PERSON_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "contacts.pb.h"
#include "gui/widgets/control.hpp"
#include "game_config.hpp"

namespace gui2 {

class tlistbox;
class ttoggle_panel;
class ttext_box;

class tedit_person: public tdialog
{
public:
	explicit tedit_person(const pb::tcontacts& contacts, pb::tperson& person, bool edit, const std::set<std::string>& bh_fields, bool read_only);

	const pb::tperson& get_person() const { return temp_person_; }

private:
	/** Inherited from tdialog. */
	void pre_show() override;

	/** Inherited from tdialog. */
	void post_show() override;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	void fill_mobiles_listbox(tlistbox& listbox);
	void save(twindow& window);

	bool mobile_number_ok(const std::string& number) const;
	bool can_save();

	void insert_mobile(twindow& window);
	void erase_mobile(tlistbox& listbox, tcontrol& widget);
	void did_number_extra_menu(ttext_box& widget, std::vector<tfloat_widget*>& menu);
	void did_longpress_menu(tlistbox& list, ttoggle_panel& row, std::vector<tfloat_widget*>& menu);
	void did_number_changed(tlistbox& listbox, ttoggle_panel& row, ttext_box& widget);
	void did_field_changed();

	pb::tperson_bh_field* find_bh_field(const std::string& id);

	void float_widget_click_erase(tlistbox& mobiles);

private:
	const pb::tcontacts& contacts_;
	const bool edit_;
	pb::tperson& temp_person_;
	const std::set<std::string>& bh_fields_;
	bool read_only_;
};

} // namespace gui2

#endif

