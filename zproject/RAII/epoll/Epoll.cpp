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

void Epoll::addListeningSocket(ListeningSocket &&sock)
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
	std::cout << "Added listening socket(" << serverFD << ")" << std::endl;
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
	std::cout << "Added client(" << clientFD << ")" << std::endl;
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

// сердце управления epoll
void Epoll::handleEvents(int defaultTimeoutMs) // epoll.handleEvents(CLIENT_TIMEOUT_MS); то есть, каждые 60 секунд проверять проверять активность клиентов
{
	auto now = std::chrono::steady_clock::now();
	// определяем timeout для epoll_wait на основе активности клиентов
	int timeout = defaultTimeoutMs;

	if (!clients.empty() && defaultTimeoutMs != 0)
	{
		// находим ближайший таймаут среди клиентов
		auto minTimeout = std::chrono::milliseconds::max();
		for (auto &pair : clients)
		{
			Client &client = pair.second;
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - client.getLastActivity());
			int remaining = CLIENT_TIMEOUT_MS - elapsed.count();
			if (remaining < 0)
				remaining = 0;
			if (remaining < minTimeout.count())
				minTimeout = std::chrono::milliseconds(remaining);
		}
		timeout = minTimeout.count();
	}

	// Вызываем epoll_wait
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
		bool isListening = false;
		for (auto &ls : listeningSockets)
		{
			if (ls.getFD() == fd)
			{
				isListening = true;
				break;
			}
		}
		if (isListening)
		{
			// accept для всех клиентов, пока accept не вернет -1 и errno = EAGAIN/EWOULDBLOCK
			while (true)
			{
				ClientSocket csock(fd); // моя обертка acceptClient внутри конструктора
				if (csock.getFD() == -1)
					break;
				Client client(std::move(csock));
				addClient(std::move(client)); // добавляем в epoll + map
			}
		}
		else
		{
			// это клиент
			Client *client = getClientByFD(fd);
			if (!client)
				continue;
			
			if (ev & (EPOLLERR | EPOLLHUP))
			{
				removeClient(fd);
				continue;
			}

			if (ev & EPOLLIN)
			{
				ssize_t bytes = client->readFromSocket();
				if (bytes == 0)
				{
					removeClient(fd);
					continue;
				}
				else
					client->updateLastActivity();
				std::cout << "Client(" << client->getFD() << "): " << client->getReadBuffer() << std::endl;
			}

			if (ev & EPOLLOUT)
			{
				if (client->hasPendingWrite())
				{
					client->writeToSocket();
					client->updateLastActivity();
				}
			}
			updateClientEvents(*client); // синхронизируем EPOLLIN/EPOLLOUT с текущим состоянием
		}
	}

	// проверка таймаутов клиентов
	now = std::chrono::steady_clock::now();
	std::vector<int> toRemove; // отдельный вектор куда мы добавим клиентов для удаления, и потом с помощью него будем удалить клиентов
	for (auto &pair : clients)
	{
		Client &client = pair.second;
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - client.getLastActivity());
		if (elapsed.count() >= CLIENT_TIMEOUT_MS)
			toRemove.push_back(client.getFD());
	}
	for (int fd : toRemove)
		removeClient(fd);
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