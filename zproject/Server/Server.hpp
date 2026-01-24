#pragma once

#include "Epoll.hpp"
#include "ServerConfig.hpp"
#include <csignal>

class Server
{
	Epoll epoll;
	std::vector<ServerConfig> configs; // configuration from file
	static bool running; // simmilar to a global var, but incapsulated in a class (i did this like that, cause i wanna use it for SIGINT also)

	public:
		Server(const std::string &configFile); // разбор файла и настройка слушащих сокетов
		void run(); // основной цикл событий
		static void sigintHandler(int sig); // обработка сигнала SIGINT
	private:
		void handleClient(Client &client); // парсинг запроса и формирование ответа
		void handleCGI(Client &client); // cgi
		void shutdownServer();
};
