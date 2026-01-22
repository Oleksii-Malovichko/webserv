#include "ClientSocket.hpp"

ClientSocket::ClientSocket(int serverFD)
{
	this->fd = acceptClient(serverFD);
}

ClientSocket::ClientSocket(ClientSocket &&other) noexcept : fd(other.fd)
{
	other.fd = -1;
}

ClientSocket& ClientSocket::operator=(ClientSocket &&other) noexcept
{
	if (this != &other)
	{
		if (fd != -1)
			::close(fd);
		fd = other.fd;
		other.fd = -1;
	}
	return *this;
}

ClientSocket::~ClientSocket()
{
	if (this->fd != -1)
		::close(this->fd);
}

int ClientSocket::getFD() const
{
	return this->fd;
}

void ClientSocket::closeFD()
{
	if (fd != -1)
	{
		::close(fd);
		fd = -1;
	}
}


void error(const char *err, int fd)
{
	if (fd != -1)
		::close(fd);
	throw std::runtime_error(std::string(err) + strerror(errno));
}

int acceptClient(int serverFD)
{
	int clientFD = accept(serverFD, NULL, NULL);
	if (clientFD == -1)
	{
		if (errno == EWOULDBLOCK || errno == EINTR || errno == EAGAIN)
			return -1;
		error("accept", clientFD);
	}
	
	int flags = fcntl(clientFD, F_GETFL, 0);
	if (flags == -1)
		error("F_GETFL", clientFD);
	if (fcntl(clientFD, F_SETFL, flags | O_NONBLOCK) == -1)
		error("F_SETFL", clientFD);

	return clientFD;
}
