#pragma once

#include "Epoll.hpp"
#include "ServerConfig.hpp"
#include "WebServConfig.hpp"
#include "ConfigParser.hpp"
#include "HttpResponce.hpp"
#include <csignal>
#include <algorithm>
#include <cctype>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

#define PHYSICAL_ROOT "./resources"

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
		static void sigintHandler(int sig); // to correctly stop the program
		void handleClient(Client &client); // parse the request and build responce (via handleGetRequest/Post/Delete)
		void handleGetRequest(HttpRequest &req, HttpResponce &resp, Client &client);
		void handlePostRequest(HttpRequest &req, HttpResponce &resp, Client &client);
		void handleCGI(HttpRequest &req, HttpResponce &resp, Client &client);
		void shutdownServer();
};
	
// void handleClientEcho(Client &client); // простой echo-ответ
std::vector<std::string> split(const std::string &s, const std::string &delimiter);
std::string trim(const std::string &s);
std::string toLower(const std::string &s);
bool fileExists(const std::string &path);
std::string getFileContent(const std::string &path);
bool isPathSafe(const std::string &fullPath, const std::string &root);
std::string getMimeType(const std::string& path);
void generateAutoIndex(const std::string &dirPath, HttpResponce &resp);
std::string buildFullPath(const LocationConfig *loc, const ServerConfig *server, const std::string &requestPath);
std::string getReasonPhrase(int code);
void buildError(HttpResponce &resp, int statusCode, const ServerConfig *server);
bool isMethodAllowed(const std::string method, const LocationConfig *loc);
void serveFileOrDirectory(const std::string& path, const HttpRequest &req, HttpResponce& resp, const LocationConfig* location, const ServerConfig* server);
void buildRedirect(HttpResponce &resp, const LocationConfig *loc);
std::string generateDefaultErrorPage(int code, const std::string &reason);
std::string getReasonPhrase(int code);
bool isMethodAllowed(const std::string method, const LocationConfig *loc);