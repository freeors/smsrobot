/* $Id: title_screen.cpp 48740 2011-03-05 10:01:34Z mordante $ */
/*
   Copyright (C) 2008 - 2011 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "smsrobot-lib"

#include "gui/dialogs/register.hpp"

#include "game_config.hpp"
#include "preferences.hpp"
#include "gettext.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/text_box2.hpp"
#include "formula_string_utils.hpp"
#include "help.hpp"
#include "filesystem.hpp"
#include <time.h>
#include "wml_exception.hpp"
#include "language.hpp"
#include "network.hpp"
#include "hero.hpp"

#include <boost/bind.hpp>

namespace gui2 {

REGISTER_DIALOG(smsrobot, register)

tregister::tregister(bool bind)
	: bind_(bind)
	, register_(NULL)
	, next_sms_ticks_(0)
{
}

void tregister::pre_show()
{
	window_->set_label("misc/white-background.png");

	std::stringstream ss;
	tlabel* label = find_widget<tlabel>(window_, "title", false, true);
	if (bind_) {
		ss.str("");
		ss << _("Register") << "(" << _("Bind Sesame Account") << ")";
		label->set_label(ss.str());
	}

	tbutton* button = find_widget<tbutton>(window_, "cancel", false, true);
	button->set_icon("misc/back.png");

	register_ = find_widget<tbutton>(window_, "register", false, true);
	register_->set_label(_("Register"));
	register_->set_canvas_variable("border", variant("login2-border"));
	register_->set_active(false);
	connect_signal_mouse_left_click(
		*register_
		, boost::bind(
			&tregister::register2
			, this
			, boost::ref(*window_)));

	button = find_widget<tbutton>(window_, "agreement", false, true);
	connect_signal_mouse_left_click(
		*button
		, boost::bind(
			&tregister::user_agreement
			, this
			, boost::ref(*window_)));

	utils::string_map symbols;
	symbols["agreement"] = "Sesame User Agreement";
	std::string agreement = vgettext2("I have read and agreed to $agreement", symbols);
	button->set_label(agreement);

	// email
	username_.reset(new ttext_box2(*window_, *find_widget<twidget>(window_, "username", false, true), null_str, "misc/user96.png"));
	username_->set_did_text_changed(boost::bind(&tregister::email_changed_callback, this, _1));
	username_->text_box().set_maximum_chars(22);
	username_->text_box().set_placeholder(_("Username"));

	// password
	password_.reset(new ttext_box2(*window_, *find_widget<tcontrol>(window_, "password", false, true), null_str, "misc/password.png", true, "misc/eye.png", false));
	connect_signal_mouse_left_click(
		password_->button()
		, boost::bind(
			&tregister::clear_password
			, this
			, boost::ref(*window_)));

	password_->text_box().set_maximum_chars(22);
	password_->text_box().set_placeholder(_("6-14 chars password"));
	password_->set_did_text_changed(boost::bind(&tregister::email_changed_callback, this, _1));
	// connect_signal_pre_key_press(password_.text_box(), boost::bind(&tregister::signal_handler_sdl_key_down, this, _3, _4, _5, _6, _7));

	// email
	email_.reset(new ttext_box2(*window_, *find_widget<twidget>(window_, "email", false, true), null_str, "misc/email.png"));
	email_->set_did_text_changed(boost::bind(&tregister::email_changed_callback, this, _1));
	email_->text_box().set_maximum_chars(64);
	email_->text_box().set_placeholder(_("E-mail"));

	tgrid& bonus_grid = find_widget<tgrid>(window_, "bonus_grid", false);
	bonus_grid.set_visible(twidget::INVISIBLE);
}

void tregister::post_show()
{
}

void tregister::set_register_active(const std::string& mobile, const std::string& password, const std::string& email)
{
	// register_->set_active(!mobile.empty() && utils::isinteger(mobile) && seccode.size() == seccode_size && seccode == seccode_str_ && !password.empty());
	bool active = !mobile.empty() && !password.empty();
	if (active) {
		std::vector<std::string> vsize = utils::split(email, '@');
		active = vsize.size() == 2;
	}

	register_->set_active(active);
}

void tregister::set_seccode_active(const std::string& mobile)
{
	tbutton& get_seccode_button = find_widget<tbutton>(window_, "get_seccode", false);
	get_seccode_button.set_active(!next_sms_ticks_ && !mobile.empty() && utils::isinteger(mobile));
}

void tregister::email_changed_callback(ttext_box& widget)
{
	set_register_active(username_->text_box().label(), password_->text_box().label(), email_->text_box().label());
}

void tregister::seccode_changed_callback(ttext_box& widget)
{
	set_register_active(username_->text_box().label(), null_str, password_->text_box().label());
}

void tregister::clear_password(twindow& window)
{
	password_->text_box().set_cipher(!password_->text_box().cipher());
}

void tregister::set_retval(twindow& window, int retval)
{
	window.set_retval(retval);
}

void tregister::user_agreement(twindow& window)
{
}

void tregister::get_seccode(twindow& window)
{
	http_agent agent;
	// seccode_str_ = agent.do_sms(email_->text_box().label(), sms_register);

	if (seccode_str_.size() == seccode_size) {
		find_widget<tbutton>(window_, "get_seccode", false).set_active(false);
		const int seccode_reserve_secs = 60;
		next_sms_ticks_ = SDL_GetTicks() + seccode_reserve_secs * 1000;
	}
}

void tregister::register2(twindow& window)
{
	std::string mobile = username_->text_box().label();
	std::string password = password_->text_box().label();

	http_agent agent;
	tuser user;
	if (current_user.uid == nposm) {
		// normal register
		user = agent.do_register(mobile, password, email_->text_box().label());
	} else {
		// bind
		const std::string openid = group.leader().name();
		// user = agent.do_bind(game_config::current_userid, mobile, password, game_config::current_openid_type, openid, 2);
	}
	if (user.valid()) {
		// switch to sesame account
		current_user = user;
		preferences::set_startup_login(true);

		window.set_retval(twindow::OK);
	}
}

void tregister::monitor_process()
{
	if (next_sms_ticks_) {
		tbutton& get_seccode_button = find_widget<tbutton>(window_, "get_seccode", false);

		Uint32 now = SDL_GetTicks();
		std::stringstream ss;

		if (now < next_sms_ticks_) {
			int remainder = (next_sms_ticks_ - now) / 1000 + 1;
			ss << remainder << dsgettext("rose-lib", "time^s");
		} else {
			next_sms_ticks_ = 0;
			ss << _("Get");
		}
		get_seccode_button.set_label(ss.str());
		set_seccode_active(username_->text_box().label());
	}
}

void tregister::signal_handler_sdl_key_down(bool& handled
		, bool& halt
		, const SDL_Keycode key
		, SDL_Keymod modifier
		, const Uint16 unicode)
{
#if (defined(__APPLE__) && TARGET_OS_IPHONE) || defined(ANDROID)
	if (key == SDLK_PRINTSCREEN) {
		tcontrol* ceil = find_widget<tcontrol>(window_, "login_ceil", false, true);
		ceil->set_visible(twidget::INVISIBLE);
	}
#endif

#ifdef _WIN32
	if ((key == SDLK_RETURN || key == SDLK_KP_ENTER) && !(modifier & KMOD_SHIFT)) {
		handled = true;
		halt = true;
	}
#else
	if (key == SDLK_RETURN) {
		tcontrol* ceil = find_widget<tcontrol>(window_, "login_ceil", false, true);
		ceil->set_visible(twidget::VISIBLE);

		handled = true;
		halt = true;
	}
#endif
}

} // namespace gui2

