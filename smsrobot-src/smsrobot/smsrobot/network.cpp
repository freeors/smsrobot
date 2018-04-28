#define GETTEXT_DOMAIN "smsrobot-lib"

#include "network.hpp"
#include "filesystem.hpp"
#include "gettext.hpp"
#include "wml_exception.hpp"
#include "gui/dialogs/message.hpp"
#include "json/json.h"
#include "game_config.hpp"
#include "formula_string_utils.hpp"

#include <iomanip>

#define ERRCODE_REMOTE_LOGIN		-2

const int seccode_size = 4;
const tuser null_user2;
tuser current_user;

http_agent::http_agent()
{
}

http_agent::~http_agent() 
{
	lobby->http->reset_connect();
}

std::string http_agent::form_url(const std::string& category, const std::string& task) const
{
	std::stringstream url;

	url << game_config::bbs_server.url << "/plugin.php?id=smsrobot:smsrobot&verion=1&";
	url << task;

	return url.str();
}

bool http_agent::do_prepare()
{
	lobby->http->set_host(game_config::bbs_server.host, game_config::bbs_server.port);
	return lobby->http->network_connect_dialog(false);
}

tuser http_agent::do_login(int type, const std::string& email, const std::string& password, bool quiet)
{
	tuser user;

	std::stringstream err;
	if (!do_prepare()) {
		if (!quiet) {
			err << _("Can not connect to network. Please check if the Internet is connected.");
			gui2::show_message(null_str, err.str());
		}
		return null_user2;
	}

	Json::Value json_root;
	json_root["username"] = email;
	json_root["password"] = password;
	Json::FastWriter writer;
	const std::string body2 = writer.write(json_root);

	std::stringstream body;
	body << std::setbase(16) << std::setfill('0') << std::setw(8) << body2.size();
	body << body2;

	const std::string request = lobby->http->form_request(form_url("user", "do=login"), body.str().size());

	size_t size = request.size() + body.str().size();
	char* buf2 = (char*)malloc(size);
	memcpy(buf2, request.c_str(), request.size());
	if (!body.str().empty()) {
		memcpy(buf2 + request.size(), body.str().c_str(), body.str().size());
	}

	lobby->http->network_send_dialog(buf2,  size, 0);
	free(buf2);
	buf2 = nullptr;

	lobby->http->network_receive_dialog();
	const int buf_size = lobby->http->response_size();
	if (!buf_size) {
		return null_user2;
	}
	const char* buf = lobby->http->response_buf();

	int userid = 0;
	config data;
	int content_start = tlobby::thttp_sock::http_2_cfg(buf, buf_size, data);
	if (content_start != -1 && content_start < buf_size) {
		const char* content = buf + content_start;
		int content_length = buf_size - content_start;

		try {
			Json::Reader reader;
			Json::Value json_object;
			if (!reader.parse(content, json_object)) {
				err << _("Invalid data.");
			} else {
				Json::Value& response_code = json_object["response_code"];
				int code = response_code.asInt();
				if (code == 20) {
					err << _("Mobile/Email or password error");

				} else if (code) {
					err << json_object["msg"].asString();
					if (err.str().empty()) {
						err << _("Unknown error.");
					}
				}
			}

			Json::Value& results = json_object["results"];
			if (results.isObject()) {
				Json::Value& uid = results["uid"];
				Json::Value& nick = results["nickname"];
				Json::Value& sessionid = results["sessionid"];
				Json::Value& contacts_ts = results["contacts_ts"];
				Json::Value& templates_ts = results["templates_ts"];

				user.uid = uid.asInt();
				user.nick = nick.asString();
				user.sessionid = sessionid.asString();
				user.contacts_ts = contacts_ts.asInt64();
				user.templates_ts = templates_ts.asInt64();
			}
		} catch (const Json::RuntimeError& e) {
			err << e.what();
		} catch (const Json::LogicError& e) {
			err << e.what();
		}

		if (err.str().empty() && !user.valid()) {
			err << _("Unknown error.");
		}

		if (!err.str().empty()) {
			if (!quiet) {
				gui2::show_message(null_str, err.str());
			}
			return null_user2;
		}
	}

	return user;
}

std::string handle_remote_login(int64_t timestamp)
{
	VALIDATE(current_user.valid(), null_str);

	utils::string_map symbols;
	symbols["time"] = format_time_date(timestamp);
	symbols["bbs_url"] = game_config::bbs_server.generate_furl();
	std::string err = vgettext2("Your account is logined at $time in different places. If it is not your operation, your password has been leaked. Please login $bbs_url as soon as possible to modify the password.", symbols);

	current_user.uid = nposm;
	return err;
}

bool http_agent::do_logout(int uid, const std::string& sessionid)
{
	VALIDATE(uid != nposm && !sessionid.empty(), null_str);

	std::stringstream err;
	if (!do_prepare()) {
		err << _("Can not connect to network. Please check if the Internet is connected.");
		gui2::show_message(null_str, err.str());
		return false;
	}
	
	Json::Value json_root;
	json_root["uid"] = uid;
	json_root["sessionid"] = sessionid;
	Json::FastWriter writer;
	const std::string body2 = writer.write(json_root);

	std::stringstream body;
	body << std::setbase(16) << std::setfill('0') << std::setw(8) << body2.size();
	body << body2;

	{
		std::string str = body.str();
		int ii = 0;
	}

	std::string request = lobby->http->form_request(form_url("user", "do=logout"), body.str().size());

	size_t size = request.size() + body.str().size();
	char* buf2 = (char*)malloc(size);
	memcpy(buf2, request.c_str(), request.size());
	if (!body.str().empty()) {
		memcpy(buf2 + request.size(), body.str().c_str(), body.str().size());
	}

	lobby->http->network_send_dialog(buf2, size, 0);
	free(buf2);
	buf2 = NULL;

	lobby->http->network_receive_dialog();
	const int buf_size = lobby->http->response_size();
	if (!buf_size) {
		return false;
	}
	const char* buf = lobby->http->response_buf();

	config data;
	int content_start = tlobby::thttp_sock::http_2_cfg(buf, buf_size, data);
	if (content_start != -1 && content_start < buf_size) {
		const char* content = buf + content_start;
		int content_length = buf_size - content_start;

		try {
			Json::Reader reader;
			Json::Value json_object;
			if (!reader.parse(content, json_object)) {
				err << _("Invalid data.");
			} else {
				Json::Value& response_code = json_object["response_code"];
				int code = response_code.asInt();
				if (code == ERRCODE_REMOTE_LOGIN) {
					Json::Value& timestamp = json_object["timestamp"];
					err << handle_remote_login(timestamp.asInt64());

				} else if (code) {
					err << json_object["msg"].asString();
					if (err.str().empty()) {
						err << _("Unknown error.");
					}
				}

			}
		} catch (const Json::RuntimeError& e) {
			err << e.what();
		} catch (const Json::LogicError& e) {
			err << e.what();
		}

		if (!err.str().empty()) {
			gui2::show_message(null_str, err.str());
			return false;
		}
	}

	return true;
}

tuser http_agent::do_register(const std::string& username, const std::string& password, const std::string& email)
{
	tuser user;

	std::stringstream err;
	if (!do_prepare()) {
		err << _("Can not connect to network. Please check if the Internet is connected.");
		gui2::show_message(null_str, err.str());
		return null_user2;
	}
	
	Json::Value json_root;
	json_root["username"] = username;
	json_root["password"] = password;
	json_root["email"] = email;
	Json::FastWriter writer;
	const std::string body2 = writer.write(json_root);

	std::stringstream body;
	body << std::setbase(16) << std::setfill('0') << std::setw(8) << body2.size();
	body << body2;

	const std::string request = lobby->http->form_request(form_url("user", "do=register"), body.str().size());

	size_t size = request.size() + body.str().size();
	char* buf2 = (char*)malloc(size);
	memcpy(buf2, request.c_str(), request.size());
	if (!body.str().empty()) {
		memcpy(buf2 + request.size(), body.str().c_str(), body.str().size());
	}

	lobby->http->network_send_dialog(buf2,  size, 0);
	free(buf2);
	buf2 = NULL;

	lobby->http->network_receive_dialog();
	const int buf_size = lobby->http->response_size();
	if (!buf_size) {
		return null_user2;
	}
	const char* buf = lobby->http->response_buf();

	config data;
	int content_start = tlobby::thttp_sock::http_2_cfg(buf, buf_size, data);
	if (content_start != -1 && content_start < buf_size) {
		const char* content = buf + content_start;
		int content_length = buf_size - content_start;

		try {
			Json::Reader reader;
			Json::Value json_object;
			if (!reader.parse(content, json_object)) {
				err << _("Invalid data.");
			} else {
				Json::Value& response_code = json_object["response_code"];
				int code = response_code.asInt();
				if (code == 2) {
					err << _("Mobile already registered.");

				} else if (code) {
					err << json_object["msg"].asString();
					if (err.str().empty()) {
						err << _("Unknown error.");
					}
				}
			}

			Json::Value& results = json_object["results"];
			if (results.isObject()) {
				Json::Value& uid = results["uid"];
				Json::Value& nick = results["nickname"];
				Json::Value& sessionid = results["sessionid"];
				Json::Value& contacts_ts = results["contacts_ts"];
				Json::Value& templates_ts = results["templates_ts"];

				user.uid = uid.asInt();
				user.nick = nick.asString();
				user.sessionid = sessionid.asString();
				user.contacts_ts = contacts_ts.asInt64();
				user.templates_ts = templates_ts.asInt64();
			}
		} catch (const Json::RuntimeError& e) {
			err << e.what();
		} catch (const Json::LogicError& e) {
			err << e.what();
		}

		if (err.str().empty() && !user.valid()) {
			err << _("Unknown error.");
		}

		if (!err.str().empty()) {
			gui2::show_message(null_str, err.str());
			return null_user2;
		}
	}

	return user;
}

bool http_agent::upload_pb(int uid, const std::string& sessionid, const std::map<int, int64_t>& files)
{
	VALIDATE(uid != nposm && !sessionid.empty(), null_str);
	VALIDATE(!files.empty(), null_str);

	std::stringstream err;
	if (!do_prepare()) {
		err << _("Can not connect to network. Please check if the Internet is connected.");
		gui2::show_message(null_str, err.str());
		return false;
	}
	
	Json::Value json_root;
	json_root["uid"] = uid;
	json_root["sessionid"] = sessionid;

	int appendix_size = 0;
	for (std::map<int, int64_t>::const_iterator it = files.begin(); it != files.end(); ++ it) {
		std::string file_name, ts_key, offset_key, size_key;
		if (it->first == CONTACTS_PB) {
			file_name = game_config::preferences_dir + "/contacts.pb";
		} else {
			VALIDATE(it->first == TEMPLATES_PB, null_str);
			file_name = game_config::preferences_dir + "/templates.pb";
		}
		tfile file(file_name, GENERIC_READ, OPEN_EXISTING);
		int fsize = file.read_2_data();
		if (!fsize) {
			continue;
		}
		appendix_size += fsize;
	}
	if (!appendix_size) {
		err << _("Special file is empty, can not upload.");
		gui2::show_message(null_str, err.str());
		return false;
	}
	char* appendix_data = (char*)malloc(appendix_size);
	appendix_size = 0;
	for (std::map<int, int64_t>::const_iterator it = files.begin(); it != files.end(); ++ it) {
		std::string file_name, ts_key, offset_key, size_key;
		if (it->first == CONTACTS_PB) {
			file_name = game_config::preferences_dir + "/contacts.pb";
			ts_key = "contacts_ts";
			offset_key = "contacts_offset";
			size_key = "contacts_size";
		} else {
			VALIDATE(it->first == TEMPLATES_PB, null_str);
			file_name = game_config::preferences_dir + "/templates.pb";
			ts_key = "templates_ts";
			offset_key = "templates_offset";
			size_key = "templates_size";
		}
		tfile file(file_name, GENERIC_READ, OPEN_EXISTING);
		int fsize = file.read_2_data();
		if (!fsize) {
			continue;
		}
		json_root[ts_key] = it->second;
		json_root[offset_key] = appendix_size;
		json_root[size_key] = fsize;
		memcpy(appendix_data + appendix_size, file.data, fsize);
		appendix_size += fsize;
	}
	Json::FastWriter writer;
	const std::string body2 = writer.write(json_root);

	std::stringstream body;
	body << std::setbase(16) << std::setfill('0') << std::setw(8) << body2.size();
	body << body2;

	std::string request = lobby->http->form_request(form_url("user", "do=uploadsave"), body.str().size() + appendix_size);

	size_t size = request.size() + body.str().size() + appendix_size;
	char* buf2 = (char*)malloc(size);
	memcpy(buf2, request.c_str(), request.size());
	if (!body.str().empty()) {
		memcpy(buf2 + request.size(), body.str().c_str(), body.str().size());
	}
	memcpy(buf2 + request.size() + body.str().size(), appendix_data, appendix_size);

	lobby->http->network_send_dialog(buf2, size, 0);
	free(buf2);
	buf2 = NULL;
	free(appendix_data);
	appendix_data = nullptr;

	lobby->http->network_receive_dialog();
	const int buf_size = lobby->http->response_size();
	if (!buf_size) {
		return false;
	}
	const char* buf = lobby->http->response_buf();

	config data;
	int content_start = tlobby::thttp_sock::http_2_cfg(buf, buf_size, data);
	if (content_start != -1 && content_start < buf_size) {
		const char* content = buf + content_start;

		try {
			Json::Reader reader;
			Json::Value json_object;
			if (!reader.parse(content, json_object)) {
				err << _("Invalid data.");
			} else {
				Json::Value& response_code = json_object["response_code"];
				int code = response_code.asInt();
				if (code == ERRCODE_REMOTE_LOGIN) {
					Json::Value& timestamp = json_object["timestamp"];
					err << handle_remote_login(timestamp.asInt64());

				} else if (code) {
					err << json_object["msg"].asString();
					if (err.str().empty()) {
						err << _("Unknown error.");
					}
				}
			}
		} catch (const Json::RuntimeError& e) {
			err << e.what();
		} catch (const Json::LogicError& e) {
			err << e.what();
		}

		if (!err.str().empty()) {
			gui2::show_message(null_str, err.str());
			return false;
		}
	}

	return true;
}

bool http_agent::download_pb(int uid, const std::string& sessionid, const std::set<int>& files)
{
	VALIDATE(uid != nposm && !sessionid.empty(), null_str);
	VALIDATE(!files.empty(), null_str);

	std::stringstream err;
	if (!do_prepare()) {
		err << _("Can not connect to network. Please check if the Internet is connected.");
		gui2::show_message(null_str, err.str());
		return false;
	}
	
	Json::Value json_root;
	json_root["uid"] = uid;
	json_root["sessionid"] = sessionid;

	bool has_contacts = false;
	bool has_templates = false;
	for (std::set<int>::const_iterator it = files.begin(); it != files.end(); ++ it) {
		std::string file_name, ts_key, offset_key, size_key;
		if (*it == CONTACTS_PB) {
			json_root["contacts"] = true;
			has_contacts = true;
		} else {
			VALIDATE(*it == TEMPLATES_PB, null_str);
			json_root["templates"] = true;
			has_templates = true;
		}
	}

	Json::FastWriter writer;
	const std::string body2 = writer.write(json_root);

	std::stringstream body;
	body << std::setbase(16) << std::setfill('0') << std::setw(8) << body2.size();
	body << body2;

	std::string request = lobby->http->form_request(form_url("user", "do=downloadsave"), body.str().size());

	size_t size = request.size() + body.str().size();
	char* buf2 = (char*)malloc(size);
	memcpy(buf2, request.c_str(), request.size());
	if (!body.str().empty()) {
		memcpy(buf2 + request.size(), body.str().c_str(), body.str().size());
	}

	lobby->http->network_send_dialog(buf2, size, 0);
	free(buf2);
	buf2 = NULL;

	lobby->http->network_receive_dialog();
	const int buf_size = lobby->http->response_size();
	if (!buf_size) {
		return false;
	}
	const char* buf = lobby->http->response_buf();

	config data;
	int content_start = tlobby::thttp_sock::http_2_cfg(buf, buf_size, data);
	if (content_start != -1 && content_start < buf_size) {
		const char* content = buf + content_start;
		const int content_size = buf_size - content_start;

		if (content_size <= 8) {
			gui2::show_message(null_str, _("Unknown error."));
			return false;
		}
		const std::string json_size_str(content, 8);
		char* endptr;
		int json_size = (int)strtol(json_size_str.c_str(), &endptr, 16);
		if (content_size < 8 + json_size) {
			gui2::show_message(null_str, _("Unknown error."));
			return false;
		}

		try {
			std::string json_str(content + 8, json_size);
			Json::Reader reader;
			Json::Value json_object;
			if (!reader.parse(json_str, json_object)) {
				err << _("Invalid data.");
			} else {
				Json::Value& response_code = json_object["response_code"];
				int code = response_code.asInt();
				if (code == ERRCODE_REMOTE_LOGIN) {
					Json::Value& timestamp = json_object["timestamp"];
					err << handle_remote_login(timestamp.asInt64());

				} else if (code) {
					err << json_object["msg"].asString();
					if (err.str().empty()) {
						err << _("Unknown error.");
					}
				}
			}

			Json::Value& results = json_object["results"];
			if (results.isObject()) {
				if (has_contacts) {
					Json::Value& offset = results["contacts_offset"];
					Json::Value& size = results["contacts_size"];
					int offset_ = offset.asInt();
					int size_ = size.asInt();
					tfile file(game_config::preferences_dir + "/contacts.pb", GENERIC_WRITE, CREATE_ALWAYS);
					posix_fwrite(file.fp, content + 8 + json_size + offset_, size_);
				}
				if (has_templates) {
					Json::Value& offset = results["templates_offset"];
					Json::Value& size = results["templates_size"];
					int offset_ = offset.asInt();
					int size_ = size.asInt();
					tfile file(game_config::preferences_dir + "/templates.pb", GENERIC_WRITE, CREATE_ALWAYS);
					posix_fwrite(file.fp, content + 8 + json_size + offset_, size_);
				}
			}

		} catch (const Json::RuntimeError& e) {
			err << e.what();
		} catch (const Json::LogicError& e) {
			err << e.what();
		}

		if (!err.str().empty()) {
			gui2::show_message(null_str, err.str());
			return false;
		}
	}

	return true;
}

