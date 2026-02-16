#include "Epoll.hpp"
#include "../../cgi/CgiHandler.hpp"

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

void Epoll::acceptClient(int clientFD)
{
	while (true)
	{
		ClientSocket csock(clientFD); // моя обертка acceptClient внутри конструктора
		if (csock.getFD() == -1)
			break;
		Client client(std::move(csock));
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
			removeClient(fd);
			return false;
		}
	}
	updateClientEvents(*client); // синхронизируем EPOLLIN/EPOLLOUT с текущим состоянием
	return true;
}

bool handleCGIevent(EventData* data, uint32_t ev)
{
	Client*  client = static_cast<Client*>(data->owner);
	if (!client)
	{
		std::cerr	<< RED << "client doesn't exist"
					<< " in handleCGIevent" << DEFAULT << std::endl;
		return(false);
	}

	CgiHandler* cgi_ptr = client->getCgiPtr();
	if (!cgi_ptr)
	{
		std::cerr	<< RED << "cgi_ptr doesn't exist"
					<< " in handleCGIevent" << DEFAULT << std::endl;
		return (false)
	}

	if (ev & (EPOLLER | EPOLLHUP))
	{
		//terminate cgi
		removeClient(client->getFD());
		return (false)
	}

	if (data->type == EventData:Type::CGI_STDOUT)
	{
		if (ev & EPOLLOUT)
			//cgi write
	}

	if (data->type == EventData:Type::CGI_STDIN)
	{
		if (ev & EPOLLIN)
			//cgi read
	}

	/*TODO separate the cgi states
	if (cgi->isFinished())
		client->finalizeCgiResponse()
	*/

	return (true);
}

// сердце управления epoll
/*
	This handleEvents function made before CGI fds added to the fds list
	During the test it worked properly 

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
*/

void Epoll::handleEvents(int defaultTimeoutMs)
{
	int timeout = getMinTimeout(defaultTimeoutMs);

	int n = epoll_wait(epfd, events.data(), events.size(), timeout);
	if (n == -1)
	{
		if (errno == EINTR)
			return ;
		throw std::runtime_error(std::string("epoll_wait: ") + strerror(errno));
	}

	for (int i = 0; i < n; i++)
	{
		EventData* data = static_cast<EventData*>(
			events[i].data.ptr);
		uint32_t ev = events[i].events;

		switch (data->type)
		{
			case EventData::Type::LISTEN_SOCKET:
				acceptClient(data->fd);
				break ;

			case EventData::Type::CLIENT_SOCKET:
				handleClient(data->fd, ev);
				break ;

			case EventData::Type::CGI_STDIN:
				handleCgiEvent(data->fd, ev); // this is still not correct function call
				break ;

			case EventData::Type::CGI_STDOUT:
				handleCgiEvent(data->fd, ev); // this is still not correct function call
				break ;
			
			default:
				std::cerr	<< RED << "The data type not in"
							<< " the possible list" << DEFAULT << std::endl;
				break;
		}
	}
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

	auto result = clients.emplace(clientFD, std::move(client));
	if (!result.second)
		throw std::runtime_error("Client already exists");

	Client* stored_client = &result.first->second;
	
	EventData* data = new EventData;
	data->type = EventData::Type::CLIENT_SOCKET;
	data->fd = clientFD;
	data->owner = stored_client;
	
	struct epoll_event cev;
	cev.events = EPOLLIN;

	cev.data.ptr = data;
	if (epoll_ctl(this->epfd, EPOLL_CTL_ADD, clientFD, &cev) == -1)
		throw std::runtime_error(std::string("epoll_ctl ADD(client): ") + strerror(errno));

	fdEventMap[clientFD] = data;

	// DEBUG
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	getpeername(clientFD, (struct sockaddr*)&addr, &len);
	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
	int port = ntohs(addr.sin_port);
	std::cout << "Added client(" << clientFD << ") "  << ip << ":" << port << std::endl;
}

void Epoll::removeClient(int clientFD)
{
	auto it = clients.find(clientFD);
	if (it != clients.end())
	{
		epoll_ctl(epfd, EPOLL_CTL_DEL, clientFD, nullptr);

		auto it_event = fdEventMap.find(clientFD);
		if (it_event != fdEventMap.end())
		{
			delete it_event->second;
			fdEventMap.erase(it_event);
		}

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

void Epoll::addCgiPipesToEpoll(const CgiHandler& cgi_obj, Client& client_obj)
{
	struct epoll_event cgiev;
	
	int cgi_out_read = cgi_obj.getCgiOutReadFD();
	int cgi_in_write = cgi_obj.getCgiInWriteFD();

	Client* stored_client = &client_obj;
	
	EventData* data_in = new EventData;
	data_in->type = EventData::Type::CGI_STDOUT;
	data_in->fd = cgi_out_read;
	data_in->owner = stored_client;

	cgiev.events = EPOLLIN;
	cgiev.data.ptr = data_in;
	if (epoll_ctl(this->epfd, EPOLL_CTL_ADD, cgi_out_read, &cgiev) == -1)
	{
		throw std::runtime_error(std::string(
			"epoll_ctl ADD(cgi_out_read): ") + strerror(errno));
	}

	fdEventMap[cgi_out_read] = data_in;

	EventData* data_out = new EventData;
	data_out->type = EventData::Type::CGI_STDIN;
	data_out->fd = cgi_in_write;
	data_out->owner = stored_client;

	cgiev.events = EPOLLOUT;
	cgiev.data.ptr = data_out;
	if (epoll_ctl(this->epfd, EPOLL_CTL_ADD, cgi_in_write, &cgiev) == -1)
	{
		throw std::runtime_error(std::string(
			"epoll_ctl ADD(cgi_in_write): ") + strerror(errno));
	}

	fdEventMap[cgi_in_write] = data_out;

	if (PRINT_MSG)
	{
		std::cout	<< "Added CGI pipes to epoll events:\n"
					<< "CGI IN write FD: " << cgi_in_write << "\n"
					<< "CGI OUT read FD: " << cgi_out_read<< std::endl;
	}
	
}

void Epoll::removeCgiPipesFromEpoll(const CgiHandler& cgi_obj)
{
	int cgi_in_write = cgi_obj.getCgiInWriteFD();
	int cgi_out_read = cgi_obj.getCgiOutReadFD();

	if (epoll_ctl(epfd, EPOLL_CTL_DEL, cgi_in_write, nullptr) == -1)
	{
		throw std::runtime_error(std::string("Epoll could not remove cgi in"
			" read filedescriptor: ") + strerror(errno));
	}

	auto it_event_in = fdEventMap.find(cgi_in_write);
	if (it_event_in != fdEventMap.end())
	{
		delete it_event_in->second;
		fdEventMap.erase(it_event_in);
	}


	if (epoll_ctl(epfd, EPOLL_CTL_DEL, cgi_out_read, nullptr) == -1)
	{
		throw std::runtime_error(std::string("Epoll could not remove cgi out write filedescriptor: ")
			+ strerror(errno));
	}

	auto it_event_out = fdEventMap.find(cgi_out_read);
	if (it_event_out != fdEventMap.end())
	{
		delete it_event_out->second;
		fdEventMap.erase(it_event_out);
	}

	if(PRINT_MSG)
	{
		std::cout << "Remove CGI pipes to epoll events:\n"
			  << "CGI IN write FD: " << cgi_in_write << "\n"
			  << "CGI OUT read FD: " << cgi_out_read << std::endl;
	}
}

