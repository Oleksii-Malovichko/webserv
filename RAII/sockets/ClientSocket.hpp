#pragma once

#include "../ServerConfig/ServerConfig.hpp"
// accept

class ClientSocket
{
	int fd;

	public:
		ClientSocket(int serverFD); // accept + fcntl
		ClientSocket(const ClientSocket&) = delete;
		ClientSocket(ClientSocket &&other) noexcept;
		ClientSocket& operator=(ClientSocket &&other) noexcept;
		~ClientSocket(); // close fd

		int getFD() const;
		void closeFD();
};

int acceptClient(int serverFD);
