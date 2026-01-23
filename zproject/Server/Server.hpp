#pragma once

#include "Epoll.hpp"
#include "ServerConfig.hpp"
#include <csignal>

class Server
{
	Epoll epoll;
	std::vector<ServerConfig> configs; // configuration from file
	bool running;

	public:
		Server(const std::string &configFile); // разбор файла и настройка слушащих сокетов
		void run(); // основной цикл событий
	private:
		void handleClient(Client &client); // парсинг запроса и формирование ответа
		void handleCGI(Client &client); // cgi
		void shutdownServer();
};
