#pragma once

#include "Epoll.hpp"
#include "ServerConfig.hpp"
#include "WebServConfig.hpp"
#include <csignal>

class Server
{
	Epoll epoll;
	std::vector<ServerConfig> configs; // configuration from file (удалить)
	WebServConfig webserv;
	static bool running; // simmilar to a global var, but incapsulated in a class (i did this like that, cause i wanna use it for SIGINT also)

	public:
		Server(const std::string &configFile); // разбор файла и настройка слушащих сокетов
		void run(); // основной цикл событий
	private:
		static void sigintHandler(int sig); // обработка сигнала SIGINT
		void handleClient(Client &client); // парсинг запроса и формирование ответа
		void handleCGI(Client &client); // cgi
		void shutdownServer();
		// bool parseConfigfile(const std::string &configFile);
};
