#include "Client.hpp"

// move передает управление сокетом из ClientSocket в Client (после этого в ClientSocket fd == -1, Client имеет уже этот fd)

Client::Client(ClientSocket &&sock, ServerConfig *conf) : socket(std::move(sock)), config(conf)
{
	this->state = State::READING;
	this->lastActivity = std::chrono::steady_clock::now();
	this->_cgi_obj = nullptr;
}

Client::Client(Client &&other) noexcept : socket(std::move(other.socket)), config(other.config) // переносим сокет
{
	this->readBuffer = std::move(other.readBuffer);
	this->writeBuffer = std::move(other.writeBuffer);
	this->state = other.state;
	this->lastActivity = other.lastActivity;

	other.state = State::CLOSED; // старая копия больше не активна
	// other.config = nullptr;
}

Client& Client::operator=(Client &&other) noexcept
{
	if (this != &other)
	{
		this->socket = std::move(other.socket);
		this->readBuffer = std::move(other.readBuffer);
		this->writeBuffer = std::move(other.writeBuffer);
		this->state = other.state;
		this->lastActivity = other.lastActivity;
		this->config = other.config;
		// other.config = nullptr;
	}
	return *this;
}

// errno запрещено использовать после read/write, потому неблокирующие сокеты и epoll гарантируют что ошибок не будет
ssize_t Client::readFromSocket()
{
	char buffer[4096];
	ssize_t bytes;

	bytes = read(socket.getFD(), buffer, sizeof(buffer));
	if (bytes > 0)
	{
		readBuffer.append(buffer, bytes);
		updateLastActivity();
	}
	else if (bytes == 0)
	{
		close();
		std::cout << "Client closed connection" << std::endl;
	}
	else // should never happend! (epoll гарантирует, что ошибок с read НЕ будет)
	{
		close();
		std::cerr << "Read error, closing connection" << std::endl;
	}
	return bytes;
}

ssize_t Client::writeToSocket()
{
	const char *ptr;
	ssize_t bytes;

	if (writeBuffer.empty())
		return 0;

	ptr = writeBuffer.data(); // to get the address of the first byte
	bytes = write(socket.getFD(), ptr, writeBuffer.size());
	if (bytes > 0)
	{
		writeBuffer.erase(0, bytes); // cut the buffer to lenght of the written bytes
		updateLastActivity();
		if (writeBuffer.empty())
			setState(State::READING);
	}
	else if (bytes == 0)
	{
		close();
		std::cout << "Client closed connection" << std::endl;
	}
	else
	{
		close();
		std::cerr << "Write error, closing connection" << std::endl;
	}
	return bytes;
}

bool Client::hasPendingWrite() const
{
	if (writeBuffer.empty())
		return false;
	return true;
}

void Client::appendToWriteBuffer(const std::string &data)
{
	writeBuffer.append(data);
	setState(State::WRITING); // maybe changed
}

void Client::updateLastActivity()
{
	this->lastActivity = std::chrono::steady_clock::now();
}

std::chrono::steady_clock::time_point Client::getLastActivity() const
{
	return this->lastActivity;
}

HttpRequest &Client::getRequest()
{
	return this->request;
}

void Client::close()
{
	socket.closeFD();
	state = State::CLOSED;
}

int Client::getFD() const
{
	return socket.getFD();
}

Client::State Client::getState() const
{
	return this->state;
}

void Client::setState(State newState)
{
	state = newState;
}

const std::string &Client::getReadBuffer() const
{
	return this->readBuffer;
}

const std::string &Client::getWriteBuffer() const
{
	return this->writeBuffer;
}

void Client::clearReadBuffer()
{
	this->readBuffer.clear();
}

CgiHandler* Client::getCgiPtr(void)
{
	return (this->_cgi_obj);
}

void Client::printHttpRequest(void)
{
	HttpRequest &req = this->getRequest();
	
	std::cout << "\n\nHttpRequest Information:" << std::endl;
	std::cout << "Method: " << req.method << std::endl;
	std::cout << "Path: " << req.path << std::endl;
	std::cout << "Query string: " << req.query_string << std::endl;
	std::cout << "Version: " << req.version << std::endl;
	for (auto it = req.headers.begin(); it != req.headers.end(); it++)
	{
		std::cout << "Key: " << it->first << std::endl;
		std::cout << "Value: " << it->second << std::endl;
	}
	std::cout << "Content-length: " << req.contentLength << std::endl;
	std::cout << "Body: " << req.body << std::endl;
}

ServerConfig *Client::getConfig() const
{
	return this->config;
}