#pragma once

#include "../ServerConfig/ServerConfig.hpp"
// listen

class ListeningSocket
{
	int fd;

	public:
		ListeningSocket(const std::string &ip, int port); // socket + fcntl + bind + listen
		ListeningSocket(const ListeningSocket&) = delete; // to avoid copying
		ListeningSocket(ListeningSocket &&other) noexcept;
		ListeningSocket& operator=(ListeningSocket&& other) noexcept;
		~ListeningSocket(); // close(fd)

		int getFD() const;
};

int create_socket(const std::string &ip, int port);