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
	std::unordered_map<int, std::string> error_pages;
	size_t client_max_body_size; // default: "10 MB"
	std::vector<LocationConfig> locations; // default: empty

	public:
		ServerConfig();
		
		void setPort(int p);
		int getPort() const;
		void setRoot(const std::string &r);
		const std::string &getRoot() const;
		void setIndex(const std::string &i);
		const std::string &getIndex() const;
		void setErrorPage(int code, const std::string &page);
		const std::string &getErrorPage(int code) const;
		void setClientMaxBodySize(size_t bs);
		size_t getClientMaxBodySize() const;

		void addLocation(const LocationConfig &loc);
		std::vector<LocationConfig> &getLocations();
		const std::vector<LocationConfig> &getLocations() const;

		bool isValid() const;
		~ServerConfig() = default;
};