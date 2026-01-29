#include "ListeningSocket.hpp"
// RAII for listening socket

ListeningSocket::ListeningSocket(int port)
{
	this->fd = create_socket(port);
	if (this->fd == -1)
		throw std::runtime_error("Failed to create listening socket");
}

ListeningSocket::ListeningSocket(ListeningSocket &&other) noexcept : fd(other.fd)
{
	other.fd = -1;
}

ListeningSocket& ListeningSocket::operator=(ListeningSocket&& other) noexcept
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

ListeningSocket::~ListeningSocket()
{
	if (this->fd != -1)
		::close(this->fd);
}

int ListeningSocket::getFD() const
{
	return this->fd;
}


static void error(const char *err, int fd)
{
	if (fd != -1)
		::close(fd);
	throw std::runtime_error(std::string(err) + strerror(errno));
}

int create_socket(int port)
{
	int server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_fd == -1)
		error("socket", server_fd);

	int flags = fcntl(server_fd, F_GETFL, 0);
	if (flags == -1)
		error("fcntl F_GETFL", server_fd);
	if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) == -1)
		error("fcntl F_SETFL", server_fd);
	
	int opt = 1; // для перезапуска сервера на том же порту без ошибок
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		error("setsockopt", server_fd);
	
	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
		error("bind", server_fd);
	
	if (listen(server_fd, SOMAXCONN) == -1) // максимум который поддерживает ОС
		error("listen", server_fd);

	return server_fd;
}
