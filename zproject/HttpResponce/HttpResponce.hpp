#pragma once

#include <string>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include "Client.hpp"

class HttpResponce
{
	int statusCode; // for instance: 200, 404, 500
	std::string statusMessage; // for instance: "OK", "Not Founc"
	std::string contentType; // for instance: "text/html", "application/json"
	std::string body; // for instance: HTML, JSON, text
	std::unordered_map<std::string, std::string> headers;
	public:
		HttpResponce();
		
		void setStatus(int code, const std::string &message);
		// void setContentType(const std::string &type);
		void setBody(const std::string &content);
		void setHeader(const std::string &key, const std::string &value);
		std::string serialize(const HttpRequest &req) const;
		void clear();
};