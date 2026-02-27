#include "WebServConfig.hpp"

WebServConfig::WebServConfig()
{
	this->servers = std::vector<ServerConfig>();
}

void WebServConfig::addServer(const ServerConfig &server)
{
	servers.push_back(server);
}

const std::vector<ServerConfig>& WebServConfig::getServers() const
{
	return servers;
}

std::vector<ServerConfig>& WebServConfig::getServers()
{
	return servers;
}