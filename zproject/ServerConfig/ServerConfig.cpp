#include "ServerConfig.hpp"

ServerConfig::ServerConfig()
{
	// this->ip = INADDR_ANY;
	this->ip = "";
	this->port = -1; // if no port -> error!
	this->root = ""; // if no root -> error!
	this->index = "index.html";
	// this->error_page_404 = "/errors/404.html";
	// this->error_page_505 = "/errors/505.html";
	this->client_max_body_size = 10 * 1024 * 1024; // 10 MB
}

void ServerConfig::setPort(int p)
{
	this->port = p;
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
	this->root = r;
}

const std::string &ServerConfig::getRoot() const
{
	return this->root;
}

void ServerConfig::setIndex(const std::string &i)
{
	this->index = i;
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
	this->client_max_body_size = bs;
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

std::vector<LocationConfig> &ServerConfig::getLocations()
{
	return this->locations;
}


bool ServerConfig::isValid() const
{
	if (port <= 0 || port > 65535)
	{
		std::cerr << "Invalid port: " << port << std::endl;
		return false;
	}
	if (root.empty())
	{
		std::cerr << "Server root is missing" << std::endl;
		return false;
	}
	return true;
}
