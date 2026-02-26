#pragma once

#include <string>
#include <chrono>
#include <unistd.h>
#include <fcntl.h>
#include "ClientSocket.hpp"

class CgiHandler;

struct HttpRequest
{
	std::string method;
	std::string path;
	std::string query;
	std::string pathInfo;
	std::string version = "HTTP/1.1";
	std::unordered_map<std::string, std::string> headers;
	std::string body;
	bool headersParsed = false;
	size_t contentLength = 0; // если есть тело
	int errorCode = 0;
	// keep-alive futures
	// bool keepAlive = false;
	// int keepAliveTimeoutMs = 0; // timeout for keep-alive
	// int keepAliveMaxRequests = 0; // max q of requests
	// int keepAliveRequestsDone = 0; // request done
};

class Client
{
	public:
		enum struct State
		{
			READING,
			WRITING,
			CLOSED
		};
	// public:
	// 	enum struct State
	// 	{
	// 		READING_HEADERS,
	// 		READING_BODY,
	// 		PROCESSING,
	// 		WRITING_RESPONCE
	// 	};
	private:
		ClientSocket socket;
		ServerConfig *config;
		std::string readBuffer;
		std::string writeBuffer;
		State state;
		std::chrono::steady_clock::time_point lastActivity;
		HttpRequest request;
		CgiHandler *_cgi_obj;

	public:
		// контсруктор принимает уже созданный сокет (explicit нужен чтобы не было неявных преоброзований)
		explicit Client(ClientSocket &&sock, ServerConfig *conf);
		// перемещение, копирование запрещено
		Client(const Client&) = delete;
		Client& operator=(const Client&) = delete;
		// разрешить перемещение
		Client(Client &&other) noexcept;
		Client& operator=(Client &&other) noexcept;

		std::string _http_response;

		ssize_t readFromSocket(); // чтение данных в readBuffer
		ssize_t writeToSocket(); // запись данных в writeBuffer
		bool hasPendingWrite() const; // есть ли данные для записи
		void appendToWriteBuffer(const std::string &data); // добавить данные для отправки
		void updateLastActivity(); // обновление времени активности
		std::chrono::steady_clock::time_point getLastActivity() const;
		HttpRequest &getRequest();

		// эти ф-ии нужны для epoll!
		void close(); // закрытие соединения
		int getFD() const; // получить fd для epoll
		State getState() const;
		void setState(State newState);
		const std::string &getReadBuffer() const;
		const std::string &getWriteBuffer() const;
		void clearReadBuffer();

		void printHttpRequest(void);

		CgiHandler* getCgiPtr(void);
		ServerConfig *getConfig() const;
};
