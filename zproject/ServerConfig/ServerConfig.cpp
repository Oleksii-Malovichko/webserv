#include "ServerConfig.hpp"

ServerConfig::ServerConfig()
{
	// this->ip = INADDR_ANY;
	this->ip = "";
	this->port = -1; // if no port -> error!
	this->root = ""; // if no root -> error!
	this->index = "index.html";
	this->serverName = "localhost.com";
	// this->error_page_404 = "/errors/404.html";
	// this->error_page_505 = "/errors/505.html";
	this->client_max_body_size = 10 * 1024 * 1024; // 10 MB
	hasListen = 0;
	hasRoot = 0;
	hasServerName = 0;
	hasIndex = 0;
	hasMaxBody = 0;
}

void ServerConfig::setPort(int p)
{
	if (hasListen)
		throw std::runtime_error("Duplicate listen directive");
	this->port = p;
	hasListen = 1;
}

int ServerConfig::getPort() const
{
	return this->port;
}

void ServerConfig::setIP(const std::string &ip)
{
	this->ip = ip;
}

const std::string &ServerConfig::getIP() const
{
	return this->ip;
}

void ServerConfig::setRoot(const std::string &r)
{
	if (hasRoot)
		throw std::runtime_error("Duplicate root directive");
	this->root = r;
	hasRoot = 1;
}

const std::string &ServerConfig::getRoot() const
{
	return this->root;
}

void ServerConfig::setIndex(const std::string &i)
{
	if (hasIndex)
		throw std::runtime_error("Duplicate index directive");
	this->index = i;
	hasIndex = 1;
}

const std::string &ServerConfig::getIndex() const
{
	return this->index;
}

void ServerConfig::setErrorPage(int code, const std::string &page) // тут должна быть проверка, какую страницу мы устанавливаем, 505 или 404
{
	error_pages[code] = page;
}

const std::string &ServerConfig::getErrorPage(int code) const
{
	static const std::string empty = "";

	std::unordered_map<int, std::string>::const_iterator it = error_pages.find(code);
	if (it != error_pages.end())
		return it->second;
	return empty;
}

void ServerConfig::setClientMaxBodySize(size_t bs)
{
	if (hasMaxBody)
		throw std::runtime_error("Duplicate max_body_size directive");
	this->client_max_body_size = bs;
	hasMaxBody = 1;
}

size_t ServerConfig::getClientMaxBodySize() const
{
	return this->client_max_body_size;
}

void ServerConfig::addLocation(const LocationConfig &loc)
{
	locations.push_back(loc);
}

const std::vector<LocationConfig> &ServerConfig::getLocations() const
{
	return this->locations;
}

void ServerConfig::setServerName(const std::string &serverName)
{
	if (hasServerName)
		throw std::runtime_error("Duplicate server_name directive");
	this->serverName = serverName;
	hasServerName = 1;
}

const std::string &ServerConfig::getServerName() const
{
	return this->serverName;
}

std::vector<LocationConfig> &ServerConfig::getLocations()
{
	return this->locations;
}


void ServerConfig::isValid() const
{
	if (port <= 0 || port > 65535)
	{
		std::stringstream error;
		error << "Invalid port: " << port;
		throw std::runtime_error(error.str());
	}
	if (root.empty())
		throw std::runtime_error("Server root is missing");
}
