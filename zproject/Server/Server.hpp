#pragma once

#include "Epoll.hpp"
#include "ServerConfig.hpp"

class Server
{
	Epoll epoll;
	std::vector<ServerConfig> configs; // configuration from file
	
	public:
		Server(const std::string &configFile); // разбор файла и настройка слушащих сокетов
		void run(); // основной цикл событий
	private:
		void handleClient(Client &client); // парсинг запроса и формирование ответа
		void handleCGI(Client &client); // cgi
};
