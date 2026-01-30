#pragma once

#include "LocationConfig.hpp"
#include "ServerConfig.hpp"
#include "WebServConfig.hpp"
#include <fstream>
#include <cctype>
#include <stack>

class ConfigParser
{
	WebServConfig webserv;

	enum Level
	{
		GLOBAL,
		SERVER,
		LOCATION,
	};

	public:
		ConfigParser(const std::string &configFile);
		const WebServConfig &getWebServ() const;
		~ConfigParser() = default;
	void parseServerLine(const std::string &line, ServerConfig &currentServer);
	void parseLocationLine(const std::string &line, LocationConfig &currentLocation);
};

std::string trim(const std::string &s);
std::vector<std::string> splitString(const std::string &str);
std::string removeComment(const std::string &str);