#pragma once

#include "Client.hpp"
#include "ListeningSocket.hpp"
#include <array>

#include "../../cgi/CgiHandler.hpp"

#define CLIENT_TIMEOUT_MS 60000 // клиент может жить в течении 60 секунд
#define MAX_EVENTS 128
#define PRINT_MSG 1

// class CgiHandler;

struct EventData
{
	enum struct Type
	{
		LISTEN_SOCKET,
		CLIENT_SOCKET,
		CGI_PIPE
	};

	Type type
	int fd;
	void* owner;
};

class Epoll
{
	int epfd;
	std::array<epoll_event, MAX_EVENTS> events;
	std::vector<ListeningSocket> listeningSockets;
	std::unordered_map<int, Client> clients;
	std::map<int, EventData*> fdEventMap;

	public: // тут explicit НЕ нужен, потому epoll не принимает аргументов
		Epoll();
		// запрет копирования
		Epoll(const Epoll&) = delete;
		Epoll &operator=(const Epoll&) = delete;
		// разрешаем перемещение (опционально)
		Epoll(Epoll &&other) noexcept;
		Epoll &operator=(Epoll &&other) noexcept;
		~Epoll();

		// методы для работы с listen-sockets
		void addListeningSocket(ListeningSocket &&sock); // добавление УЖЕ созданного listen-socket и регистрация в epoll
		// методы для работы с клиентами for handleEvents
		void acceptClient(int listenFD); // принять нового клиента в epoll (в цикле)
		void removeClientVec(); // удаление вектора клиентов
		void updateClientEvents(Client &client); // изменить событие EPOLLIN/EPOLLOUT в зависимости от состояния
		bool handleClient(int fd, uint32_t ev); // обработка клиентов
		// основной цикл обработки событий
		void handleEvents(int defaultTimeoutMs = -1); // вызов epoll_wait и обработка событий (время в аргументе - тик сервера)
		
		// Утилиты для Server
		int getEPFD() const; // получить epfd
		const std::vector<ListeningSocket> &getListeningSockets() const; // access to listeningSockets
		const std::unordered_map<int, Client> &getClients() const; // access to clients (reading)
		std::unordered_map<int, Client> &getClients(); // access to clients (writing/changing)

		//Add CGI pipes to epoll events
		void addCgiPipesToEpoll(const CgiHandler& cgi_obj);
		void removeCgiPipesFromEpoll(const CgiHandler& cgi_obj);

	private: // help functions
		int getMinTimeout(int defaultTimeoutMs);
		std::vector<int> getRemoveVector();
		bool checkListeningSockets(int fd);
		void addClient(Client &&client); // добавить нового клиента в epoll
		void removeClient(int clientFD); // удалить клиента и deregister из epoll
		Client *getClientByFD(int fd); // получить указатель на клиента по fd
};
