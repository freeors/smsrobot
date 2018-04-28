/* $Id: game_config.cpp 46969 2010-10-08 19:45:32Z mordante $ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"
#include "game_config.hpp"
#include "gettext.hpp"

#include <sstream>

std::string tsim_info::description() const
{
	std::stringstream ss;
	ss << carrier << "-" << number << "(L" << (level22? "22": "21") << ")";
	return ss.str();
}

std::string sim_strategy_short_description(const int sim)
{
	if (sim == sim_none) {
		return _("None");
	} else if (sim == sim_sim0) {
		return _("SIM1");
	} else if (sim == sim_sim1) {
		return _("SIM2");
	}

	VALIDATE(sim == sim_alternate, null_str);
	return _("Alternate");
}

void tparameters::push_back(const std::string& id, const std::string& name)
{
	for (std::vector<std::pair<std::string, std::string> >::const_iterator it = map.begin(); it != map.end(); ++ it) {
		VALIDATE(id != it->first, null_str);
	}
	map.push_back(std::make_pair(id, name));
}

bool tparameters::is_existed(const std::string& id) const
{
	for (std::vector<std::pair<std::string, std::string> >::const_iterator it = map.begin(); it != map.end(); ++ it) {
		if (it->first == id) {
			return true;
		}
	}
	return false;
}

const std::string& tparameters::name(const std::string& id) const
{
	for (std::vector<std::pair<std::string, std::string> >::const_iterator it = map.begin(); it != map.end(); ++ it) {
		if (it->first == id) {
			return it->second;
		}
	}
	VALIDATE(false, null_str);
	return null_str;
}

namespace game_config {
	int record_threshold = 15;
	const std::string markup_cookie_key = "field";
/*
	std::string current_user;
	int current_userid = nposm;
	int current_openid_type = 0;
	std::string current_sessionid;
*/
}


namespace preferences {

int sim_strategy()
{
	int sim = preferences::get2("sim", sim_none);
	if (sim > sim_max) {
		sim = sim_none;
	}
	return sim;
}

void set_sim_strategy(int _sim)
{
	VALIDATE(_sim <= sim_max, null_str);
	if (sim_strategy() != _sim) {
		preferences::set("sim", _sim);
		preferences::write_preferences();
	}
}

int interval()
{
	return preferences::get2("interval", 60);
}

void set_interval(int _interval)
{
	VALIDATE(_interval > 0, null_str);
	if (interval() != _interval) {
		preferences::set("interval", _interval);
		preferences::write_preferences();
	}
}

int next_send_at()
{
	return preferences::get2("next_send_at", 0);
}

void set_next_send_at(int _next_send_at)
{
	VALIDATE(_next_send_at >= 0, null_str);
	if (next_send_at() != _next_send_at) {
		preferences::set("next_send_at", _next_send_at);
		preferences::write_preferences();
	}
}

std::string bbs_server_furl()
{
	return preferences::get("bbs_server_furl");
}

void set_bbs_server_furl(const std::string& furl)
{
	VALIDATE(!furl.empty(), null_str);
	if (bbs_server_furl() != furl) {
		preferences::set("bbs_server_furl", furl);
		preferences::write_preferences();
	}
}

}