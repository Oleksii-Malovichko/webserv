#include "ServerConfig.hpp"

ServerConfig::ServerConfig()
{
	this->port = -1; // if no port -> error!
	this->root = ""; // if no root -> error!
	this->index = "index.html";
	this->error_page_404 = "/errors/404.html";
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