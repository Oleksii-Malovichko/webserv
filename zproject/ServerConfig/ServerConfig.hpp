#pragma once

#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <unordered_map>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "../LocationConfig/LocationConfig.hpp"

class ServerConfig
{
	int port; // listen, обязателен
	std::string root; // root, обязателен (нужен будет когда буду строить ответы: GET, POST, DELETE)

	// Необязательные параметры с дефолтами
	std::string index; // default: "index.html"
	std::string error_page_404; // default: "/errors/404.html"
	size_t client_max_body_size; // default: "10 MB"
	std::vector<LocationConfig> locations; // default: empty

	public:
		ServerConfig();
		void setPort(int p);
		int getPort() const;
};