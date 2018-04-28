#ifndef GUI_DIALOGS_EDIT_FIELD_HPP_INCLUDED
#define GUI_DIALOGS_EDIT_FIELD_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "contacts.pb.h"
#include "game_config.hpp"

namespace gui2 {

class tedit_field: public tdialog
{
public:
	explicit tedit_field(const pb::tcontacts& contacts, pb::tbh_field& bh_field, bool edit, const tparameters& parameters);

	const pb::tbh_field& get_bh_field() const { return temp_bh_field_; }

private:
	/** Inherited from tdialog. */
	void pre_show() override;

	/** Inherited from tdialog. */
	void post_show() override;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	void save(twindow& window);
	bool can_save();
	void did_field_changed();

private:
	bool edit_;
	pb::tbh_field& temp_bh_field_;
	const pb::tcontacts& contacts_;
	const tparameters& parameters_;

	std::set<std::string> used_ids_;
};

} // namespace gui2

#endif

