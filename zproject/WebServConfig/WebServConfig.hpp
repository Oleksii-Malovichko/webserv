#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "ServerConfig/ServerConfig.hpp"

class WebServConfig
{
	std::vector<ServerConfig> servers;
	public:
		WebServConfig();
		void addServer(const ServerConfig &server);
		const std::vector<ServerConfig>& getServers() const;
};
