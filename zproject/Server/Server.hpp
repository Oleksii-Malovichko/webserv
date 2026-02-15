#pragma once

#include "Epoll.hpp"
#include "ServerConfig.hpp"
#include "WebServConfig.hpp"
#include "ConfigParser.hpp"
#include "HttpResponce.hpp"
#include <csignal>
#include <algorithm>
#include <cctype>

class Server
{
	// std::vector<ServerConfig> configs; // configuration from file (удалить)
	Epoll epoll;
	WebServConfig webserv;
	static bool running; // simmilar to a global var, but incapsulated in a class (i did this like that, cause i wanna use it for SIGINT also)

	public:
		Server(const std::string &configFile); // разбор файла и настройка слушащих сокетов
		void run(); // основной цикл событий
	private:
		static void sigintHandler(int sig); // обработка сигнала SIGINT
		void handleClient(Client &client); // парсинг запроса и формирование ответа
		void handleParseRequest(Client &client);
		void handleCGI(Client &client); // cgi
		void shutdownServer();
};
	
// void handleClientEcho(Client &client); // простой echo-ответ
std::vector<std::string> split(const std::string &s, const std::string &delimiter);
std::string trim(const std::string &s);