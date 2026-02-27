#pragma once

#include <string>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include "Client.hpp"

class HttpResponce
{
	int statusCode; // for instance: 200, 404, 500
	std::string statusMessage; // for instance: "OK", "Not Found"
	std::string body; // for instance: HTML, JSON, text
	std::string serverName;
	std::unordered_map<std::string, std::string> headers;

	public:
		HttpResponce();
		~HttpResponce() = default;

		void setStatus(int code, const std::string &message);
		void setBody(const std::string &content);
		void setHeader(const std::string &key, const std::string &value);
		void setServerName(const std::string &serverName);
		std::string serialize(const HttpRequest &req) const;
		void clear();
};