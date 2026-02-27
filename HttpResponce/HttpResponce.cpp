#include "HttpResponce.hpp"

HttpResponce::HttpResponce()
{
	statusCode = 200; // default value
	statusMessage = "OK"; // default value
}

void HttpResponce::setStatus(int code, const std::string &message)
{
	statusCode = code;
	statusMessage = message;
}

void HttpResponce::setBody(const std::string &content)
{
	body = content;
}

void HttpResponce::setHeader(const std::string &key, const std::string &value)
{
	headers[key] = value;
}

void HttpResponce::setServerName(const std::string &serverName)
{
	this->serverName = serverName;
}

std::string HttpResponce::serialize(const HttpRequest &req) const
{
	std::stringstream responce;

	responce << req.version << " " << statusCode << " " << statusMessage << "\r\n";
	responce << "Server: " << serverName << "\r\n";
	responce << "Content-Length: " << body.size() << "\r\n";
	for (const auto &header : headers)
	{
		responce << header.first << ": " << header.second << "\r\n";
	}
	responce << "\r\n";
	responce << body;
	return responce.str();
}

void HttpResponce::clear()
{
	statusCode = 200;
	statusMessage = "OK";
	body.clear();
	headers.clear();
	// *this = HttpResponce();
}
