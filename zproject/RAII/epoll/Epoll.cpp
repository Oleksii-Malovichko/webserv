#include "Epoll.hpp"

Epoll::Epoll()
{
	epfd = epoll_create1(0);
	if (epfd == -1)
		throw std::runtime_error(std::string("epoll_create1: ") + strerror(errno));
}

Epoll::Epoll(Epoll &&other) noexcept
{
	this->epfd = other.epfd;
	other.epfd = -1;
	this->events = std::move(other.events);
	this->listeningSockets = std::move(other.listeningSockets);
	this->clients = std::move(other.clients);
}

Epoll &Epoll::operator=(Epoll &&other) noexcept
{
	if (this != &other)
	{
		if (epfd != -1)
			::close(epfd);
		this->epfd = other.epfd;
		other.epfd = -1;
		this->events = std::move(other.events);
		this->listeningSockets = std::move(other.listeningSockets);
		this->clients = std::move(other.clients);
	}
	return *this;
}

Epoll::~Epoll()
{
	if (epfd != -1)
		::close(epfd);
}

void Epoll::addListeningSocket(ListeningSocket &&sock, ServerConfig *config)
{
	int serverFD = sock.getFD();
	if (serverFD == -1)
		throw std::logic_error("addListeningSocket: invalid fd");

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = serverFD;
	if (epoll_ctl(this->epfd, EPOLL_CTL_ADD, serverFD, &ev) == -1)
		throw std::runtime_error(std::string("epoll_ctl ADD(server): ") + strerror(errno));
	
	listeningSockets.push_back(std::move(sock));
	listeningFDs[serverFD] = config;
	std::cout << "Added listening socket(" << serverFD << ")" << std::endl;
}

ListeningSocket *Epoll::getListeningSocketByFD(int fd)
{
	for (size_t i = 0; i < listeningSockets.size(); i++)
	{
		if (listeningSockets[i].getFD() == fd)
			return &listeningSockets[i];
	}
	return NULL;
}

void Epoll::acceptClient(int serverFD)
{
	ServerConfig *config = listeningFDs[serverFD];
	if (!config)
		return ;
	std::cout << "DEBBUG: Epoll:acceptClient:\n";
	std::cout << "Port: " << config->getPort() << std::endl;
	while (true)
	{
		ClientSocket csock(serverFD); // моя обертка acceptClient внутри конструктора
		if (csock.getFD() == -1)
			break;
		Client client(std::move(csock), config);
		addClient(std::move(client)); // добавляем клиента в map от epoll
	}
}

void Epoll::removeClientVec()
{
	std::vector<int> removeVec = getRemoveVector();
	for (int fd : removeVec)
	{
		removeClient(fd);
		std::cout << "Client(" << fd << ") was removed cause of timeout" << std::endl;
	}
}

void Epoll::updateClientEvents(Client &client)
{
	int fd = client.getFD();
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = 0;
	// то есть тут мы получаем state клиента и меняем его в epollin
	if (client.getState() == Client::State::READING)
		ev.events |= EPOLLIN;
	else if (client.getState() == Client::State::WRITING && client.hasPendingWrite())
		ev.events |= EPOLLOUT;

	if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1)
		throw std::runtime_error(std::string("epoll_ctl MOD: ") + strerror(errno));
}

bool Epoll::handleClient(int fd, uint32_t ev)
{
	Client *client = getClientByFD(fd);
	if (!client)
		return false;
	if (ev & (EPOLLERR | EPOLLHUP)) // ошибка
	{
		removeClient(fd);
		return false;
	}
	if (ev & EPOLLIN) // reading
	{
		ssize_t bytes = client->readFromSocket();
		if (bytes == 0) // клиент отключился
		{
			removeClient(fd);
			return false;
		}
		else
			client->updateLastActivity();
		// std::cout << "Client(" << client->getFD() << "): " << client->getReadBuffer() << std::endl;
	}
	if (ev & EPOLLOUT)
	{
		if (client->hasPendingWrite())
		{
			client->writeToSocket();
			client->updateLastActivity();
		}
		if (!client->hasPendingWrite()) // если мы уже все отправили клиенту - закрываем его
		{
			// if (!client->isKeepAlive() || client->getKeepAliveRequests() >= client->getKeepAliveMaxRequests())
			// {
			// 	removeClient(fd); // закрываем соединение
			// }
			removeClient(fd);
			return false;
		}
	}
	updateClientEvents(*client); // синхронизируем EPOLLIN/EPOLLOUT с текущим состоянием
	return true;
}

// сердце управления epoll
void Epoll::handleEvents(int defaultTimeoutMs) // epoll.handleEvents(CLIENT_TIMEOUT_MS); то есть, каждые 60 секунд проверять проверять активность клиентов
{
	int timeout = getMinTimeout(defaultTimeoutMs);

	int n = epoll_wait(epfd, events.data(), events.size(), timeout);
	if (n == -1)
	{
		if (errno == EINTR)
			return ;
		throw std::runtime_error(std::string("epoll_wait: ") + strerror(errno));
	}

	// перебираем все события
	for (int i = 0; i < n; i++)
	{
		int fd = events[i].data.fd;
		uint32_t ev = events[i].events;

		// если это listen-socket -> accept клиентов
		bool isListening = checkListeningSockets(fd);
		if (isListening)
			acceptClient(fd); // accept для всех клиентов, пока accept не вернет -1 и errno = EAGAIN/EWOULDBLOCK
		else
		{
			if (!handleClient(fd, ev))
				continue;
		}
	}
	// получить веткор для удаления клиентов (по истечению таймаутов)
	removeClientVec();
}

int Epoll::getEPFD() const
{
	return this->epfd;
}

const std::vector<ListeningSocket> &Epoll::getListeningSockets() const
{
	return this->listeningSockets;
}

const std::unordered_map<int, Client> &Epoll::getClients() const // only for reading
{
	return this->clients;
}

std::unordered_map<int, Client> &Epoll::getClients() // for writing and changing (for example closing the fd of client)
{
	return this->clients;
}

// private
// получить минимальный таймаут у клиента для epoll_wait, чтобы проснутся во время и удалить клиента
int Epoll::getMinTimeout(int defaultTimeoutMs)
{
	auto now = std::chrono::steady_clock::now(); // получить тек время
	int timeout = defaultTimeoutMs; // базовый таймаут, который мы исп пока нет клиентов
	// без таймаута в epoll_wait, я не смогу проверять таймаут клиентов вовремя
	if (!clients.empty() && defaultTimeoutMs != 0)
	{
		auto minTimeout = std::chrono::milliseconds::max(); // берем самое большое число мс, чтобы оно было гарантировано больше всех таймаутов клиентов
		for (auto &pair : clients)
		{
			Client &client = pair.second;
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - client.getLastActivity()); // (истекло). Сколько клиент уже молчит
			int remaining = CLIENT_TIMEOUT_MS - elapsed.count(); // сколько клиенту осталось жить
			if (remaining < 0) // если мы опоздали и время истекло, то ставим 0 (защита от отрицательных чисел)
				remaining = 0;
			if (remaining < minTimeout.count()) // count() нужен для перевода объекта milliseconds в int/long
				minTimeout = std::chrono::milliseconds(remaining);
		}
		timeout = minTimeout.count();
	}
	return timeout;
}

std::vector<int> Epoll::getRemoveVector()
{
	auto now = std::chrono::steady_clock::now();
	std::vector<int> toRemove;
	
	for (auto &pair : clients)
	{
		Client &client = pair.second;
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - client.getLastActivity());
		if (elapsed.count() > CLIENT_TIMEOUT_MS)
			toRemove.push_back(pair.first); // add fd of the client
	}
	return toRemove;
}

bool Epoll::checkListeningSockets(int fd)
{
	bool isListening = false;
	for (auto &ls : listeningSockets)
	{
		if (ls.getFD() == fd)
		{
			isListening = true;
			break;
		}
	}
	return isListening;
}

void Epoll::addClient(Client &&client)
{
	int clientFD = client.getFD();
	if (clientFD == -1)
		throw std::logic_error("addClient: invalid fd");
	
	struct epoll_event cev;
	cev.events = EPOLLIN;
	cev.data.fd = clientFD;
	if (epoll_ctl(this->epfd, EPOLL_CTL_ADD, clientFD, &cev) == -1)
		throw std::runtime_error(std::string("epoll_ctl ADD(client): ") + strerror(errno));
	
	clients.emplace(clientFD, std::move(client));

	// DEBUG

	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	getpeername(clientFD, (struct sockaddr*)&addr, &len);
	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
	int port = ntohs(addr.sin_port);
	std::cout << "Added client(" << clientFD << ") "  << ip << ":" << port << std::endl;
	//
	ServerConfig *config = client.getConfig();
	std::cout << "DEBUG: Epoll::addClient\n";
	std::cout << "Port: " << config->getPort() << std::endl; 
	// std::cout << "Debug with added ServerConfig to client: ip: " << config->getIP() << "; port: " << config->getPort() << std::endl;
}

void Epoll::removeClient(int clientFD)
{
	auto it = clients.find(clientFD);
	if (it != clients.end())
	{
		epoll_ctl(epfd, EPOLL_CTL_DEL, clientFD, nullptr);
		it->second.close(); // так как мы переместили client уже в этот класс, то и закрываем его здесь
		clients.erase(it); // удалить из map
	}
}

Client *Epoll::getClientByFD(int fd)
{
	auto it = clients.find(fd);
	if (it != clients.end())
		return &it->second; // return Client address
	return nullptr;
}
