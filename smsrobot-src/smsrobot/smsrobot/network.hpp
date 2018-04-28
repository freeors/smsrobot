#ifndef NETWORK_HPP_INCLUDED
#define NETWORK_HPP_INCLUDED

#include "lobby.hpp"

extern const int seccode_size;

struct tuser {
	tuser(int uid = -1, const std::string& nick = "", const std::string& sessionid = "", int64_t contacts_ts = 0, int64_t templates_ts = 0, const std::string& avatar = "misc/user.png")
		: uid(uid)
		, nick(nick)
		, sessionid(sessionid)
		, contacts_ts(contacts_ts)
		, templates_ts(templates_ts)
		, avatar(avatar)
	{}

	bool valid() const { return uid != -1; }

	int uid;
	std::string nick;
	std::string sessionid;
	int64_t contacts_ts;
	int64_t templates_ts;
	std::string avatar;
};

extern const tuser null_user2;
extern tuser current_user;

class thttp_sock2: public tlobby::thttp_sock
{
public:
	// std::string form_url(const std::string& task) const;
};

class http_agent
{
public:
	http_agent();
	~http_agent();

	bool do_prepare();
	tuser do_login(int type, const std::string& mobile, const std::string& password, bool quiet);
	bool do_logout(int uid, const std::string& sessionid);
	tuser do_register(const std::string& username, const std::string& password, const std::string& email);
	bool upload_pb(int uid, const std::string& sessionid, const std::map<int, int64_t>& files);
	bool download_pb(int uid, const std::string& sessionid, const std::set<int>& files);

private:
	std::string form_url(const std::string& category, const std::string& task) const;
};


#endif
