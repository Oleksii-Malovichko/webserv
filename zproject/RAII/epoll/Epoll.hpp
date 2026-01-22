#pragma once

#include "Client.hpp"
#include "ListeningSocket.hpp"
#include <array>

#define CLIENT_TIMEOUT_MS 60'000
#define MAX_EVENTS 128

class Epoll
{
	int epfd;
	std::array<epoll_event, MAX_EVENTS> events;
	// когда мне это заполнить? в конструкторе?
	std::vector<ListeningSocket> listeningSockets;
	// std::vector<Client> clients;
	std::unordered_map<int, Client> clients;

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
	
		// методы для работы с клиентами
		void addClient(Client &&client); // добавить нового клиента в epoll
		void removeClient(int clientFD); // удалить клиента и deregister из epoll
		Client *getClientByFD(int fd); // получить указатель на клиента по fd
	
		// основной цикл обработки событий
		void handleEvents(int timeout = -1); // вызов epoll_wait и обработка событий
		void updateClientEvents(Client &client); // изменить событие EPOLLIN/EPOLLOUT в зависимости от состояния

		// Утилиты
		int getEPFD() const; // получить epfd
		const std::vector<ListeningSocket> &getListeningSockets() const;
		const std::unordered_map<int, Client> &getClients() const; // доступ к вектору клиентов
};
