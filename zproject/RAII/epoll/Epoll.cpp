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

void Epoll::addListeningSocket(ListeningSocket &&sock, ServerConfig *config)
{
	int serverFD = sock.getFD();
	if (serverFD == -1)
		throw std::logic_error("addListeningSocket: invalid fd");
		
	struct epoll_event ev;

	// EventData* data = new EventData;
	// data->type = EventData::Type::LISTEN_SOCKET;
	// data->fd = serverFD;
	
	ev.events = EPOLLIN;

	// listeningSockets.push_back(std::move(sock));
	// data->owner = &listeningSockets.back();
	// ev.data.ptr = data;
	ev.data.fd = serverFD;

	if (epoll_ctl(this->epfd, EPOLL_CTL_ADD, serverFD, &ev) == -1)
		throw std::runtime_error(std::string("epoll_ctl ADD(server): ") + strerror(errno));
	

	// fdEventMap[serverFD] = data;

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

	// auto it_event = fdEventMap.find(fd);
	// if (it_event != fdEventMap.end())
	// {
	// 	delete it_event->second;
	// 	fdEventMap.erase(it_event);
	// }

	// EventData* data = new EventData;
	// data->type = EventData::Type::CLIENT_SOCKET;
	// data->fd = fd;
	// data->owner = &client;

	// ev.data.ptr = data;

	std::cerr	<< YELLOW << "The update client event function called for client"
				<< fd << DEFAULT << std::endl;
	// то есть тут мы получаем state клиента и меняем его в epollin
	if (client.getState() == Client::State::READING)
	{
		std::cerr	<< BLUE << "The event changed EPOLLIN"
					<< DEFAULT << std::endl;
		ev.events |= EPOLLIN;
	}
	else if (client.getState() == Client::State::WRITING && client.hasPendingWrite())
	{
		std::cerr	<< CYAN << "The event changed EPOLLOUT"
					<< DEFAULT << std::endl;
		ev.events |= EPOLLOUT;
	}

	if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1)
		throw std::runtime_error(std::string("epoll_ctl MOD: ") + strerror(errno));

	// fdEventMap[fd] = data;
}

bool Epoll::handleClient(int fd, uint32_t ev)
{
	Client *client = getClientByFD(fd);
	if (!client)
		return false;
	if (ev & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) // ошибка
	{
		removeClient(fd);
		std::cerr << RED << "Error occured handleClient EPOLLER" << DEFAULT << std::endl; 
		return false;
	}
	if (ev & EPOLLIN) // reading
	{
		ssize_t bytes = client->readFromSocket();
		if (bytes == 0) // клиент отключился
		{
			removeClient(fd);
			std::cerr << RED << "Error occured handleClient EPOLLIN" << DEFAULT << std::endl; 
			return false;
		}
		else
			client->updateLastActivity();
		
		std::cout << "Client(" << client->getFD() << ") read: " << client->getReadBuffer() << std::endl;
	}
	if (ev & EPOLLOUT)
	{
		std::cout << "Client(" << client->getFD() << ") write: " << client->getWriteBuffer() << std::endl;
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

bool Epoll::handleCgiEvent(EventData* data, uint32_t ev)
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
		return (false);
	}

	if (ev & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
	{
		cgi_ptr->terminateChild();
		//it should later send 500 error message
		removeClient(client->getFD());
		return (false);
	}

	if (data->type == EventData::Type::CGI_STDIN)
	{
		if (ev & EPOLLOUT)
			cgi_ptr->writeToCgi(*this, client->getRequest().body);
	}

	if (data->type == EventData::Type::CGI_STDOUT)
	{
		if (ev & EPOLLIN)
			cgi_ptr->readFromCgi(*this);
	}

	if (cgi_ptr->IsCgiFinished() == true)
	{
		client->_http_response = cgi_ptr->buildCgiResponse();
		client->appendToWriteBuffer(client->_http_response);
		client->clearReadBuffer();
		client->setState(Client::State::WRITING);
		updateClientEvents(*client);
	}

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

void Epoll::dataEventCheck(EventData* data)
{
	if (!data)
	{
		std::cerr << RED << "NULL data.ptr" << DEFAULT << std::endl;
	}

	std::cerr << CYAN
			<< "data ptr        = " << static_cast<void*>(data) << "\n"
			<< "data->fd        = " << data->fd << "\n"
			<< "data->type      = " << static_cast<int>(data->type)
			<< DEFAULT << std::endl;

	if (fdEventMap.find(data->fd) == fdEventMap.end())
	{
		std::cerr << RED << "fd not in fdEventMap -> invalid pointer"
				<< DEFAULT << std::endl;
	}
}

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

	// std::cerr << YELLOW << "n = " << n << "\n EventMap elements: "
	// 			<< fdEventMap.size() << DEFAULT << std::endl;  

	for (int i = 0; i < n; i++)
	{
		this->printEvenMap();

		// EventData* data = static_cast<EventData*>(
		// 	events[i].data.ptr);
		// uint32_t ev = events[i].events;

		// dataEventCheck(data);

		// switch (data->type)
		// {
		// 	case EventData::Type::LISTEN_SOCKET:
		// 		std::cerr << MAGENTA << "AcceptClient" << DEFAULT << std::endl;
		// 		acceptClient(data->fd);
		// 		break ;

		// 	case EventData::Type::CLIENT_SOCKET:
		// 		std::cerr << MAGENTA << "HandleClient" << DEFAULT << std::endl;
		// 		handleClient(data->fd, ev);
		// 		break ;

		// 	case EventData::Type::CGI_STDIN:
		// 		std::cerr << MAGENTA << "CGI_STDIN" << DEFAULT << std::endl;
		// 		handleCgiEvent(data, ev); // this is still not correct function call
		// 		break ;

		// 	case EventData::Type::CGI_STDOUT:
		// 		std::cerr << MAGENTA << "CGI_STDOUT" << DEFAULT << std::endl;
		// 		handleCgiEvent(data, ev); // this is still not correct function call
		// 		break ;
			
		// 	default:
		// 		std::cerr	<< RED << "The data type not in"
		// 					<< " the possible list" << DEFAULT << std::endl;
		// 		break;
		// }

		int fd = events[i].data.fd;
		uint32_t ev = events[i].events;
		
		bool isListening = checkListeningSockets(fd);
		if (isListening)
			acceptClient(fd); // accept для всех клиентов, пока accept не вернет -1 и errno = EAGAIN/EWOULDBLOCK
		else
		{
			if (!handleClient(fd, ev))
				continue;
		}
	}
	removeClientVec();
}

int Epoll::getEPFD() const
{
	return this->epfd;
}

// const std::vector<ListeningSocket> &Epoll::getListeningSockets() const
// {
// 	return this->listeningSockets;
// }

const std::deque<ListeningSocket> &Epoll::getListeningSockets() const
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

	// Client* stored_client = &result.first->second;
	
	// EventData* data = new EventData;
	// data->type = EventData::Type::CLIENT_SOCKET;
	// data->fd = clientFD;
	// data->owner = stored_client;
	
	struct epoll_event cev;
	cev.events = EPOLLIN;

	// cev.data.ptr = data;
	cev.data.fd = clientFD;
	if (epoll_ctl(this->epfd, EPOLL_CTL_ADD, clientFD, &cev) == -1)
		throw std::runtime_error(std::string("epoll_ctl ADD(client): ") + strerror(errno));

	// cev.data.ptr = data;
	// fdEventMap[clientFD] = data;

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
	std::cerr << GREEN << "The following Client fd removed: "
				<< clientFD << DEFAULT << std::endl;
	if (it != clients.end())
	{
		epoll_ctl(epfd, EPOLL_CTL_DEL, clientFD, nullptr);

		// auto it_event = fdEventMap.find(clientFD);
		// if (it_event != fdEventMap.end())
		// {
		// 	delete it_event->second;
		// 	fdEventMap.erase(it_event);
		// }

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
	
	EventData* data_out = new EventData;
	data_out->type = EventData::Type::CGI_STDOUT;
	data_out->fd = cgi_out_read;
	data_out->owner = stored_client;

	cgiev.events = EPOLLIN;
	cgiev.data.ptr = data_out;
	if (epoll_ctl(this->epfd, EPOLL_CTL_ADD, cgi_out_read, &cgiev) == -1)
	{
		throw std::runtime_error(std::string(
			"epoll_ctl ADD(cgi_out_read): ") + strerror(errno));
	}

	fdEventMap[cgi_out_read] = data_out;

	EventData* data_in = new EventData;
	data_in->type = EventData::Type::CGI_STDIN;
	data_in->fd = cgi_in_write;
	data_in->owner = stored_client;

	cgiev.events = EPOLLOUT;
	cgiev.data.ptr = data_in;
	if (epoll_ctl(this->epfd, EPOLL_CTL_ADD, cgi_in_write, &cgiev) == -1)
	{
		throw std::runtime_error(std::string(
			"epoll_ctl ADD(cgi_in_write): ") + strerror(errno));
	}

	fdEventMap[cgi_in_write] = data_in;

	if (PRINT_MSG)
	{
		std::cout	<< "Added CGI pipes to epoll events:\n"
					<< "CGI IN write FD: " << cgi_in_write << "\n"
					<< "CGI OUT read FD: " << cgi_out_read<< std::endl;
	}
	
}

// This function later won't need
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

void Epoll::removeCgiFd(int fd)
{
	if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr) == -1)
	{
		throw std::runtime_error(std::string("Epoll could not remove the"
			" given " + std::to_string(fd) + " filedescriptor: ") 
			+ strerror(errno));
	}

	auto it_event_in = fdEventMap.find(fd);
	if (it_event_in != fdEventMap.end())
	{
		delete it_event_in->second;
		fdEventMap.erase(it_event_in);
	}

	if(PRINT_MSG)
	{
		std::cout	<< "Remove the following fd to epoll events:\n"
					<< "filedescriptor: " << fd << std::endl;
	}
}

void Epoll::printEvenMap(void)
{
	for (auto it = fdEventMap.begin(); 
		it != fdEventMap.end(); ++it)
	{
		EventData* data = it->second; 
		
		std::cout << MAGENTA
				<< "data ptr        = " << static_cast<void*>(data) << "\n"
				<< "data->fd        = " << data->fd << "\n"
				<< "data->type      = " << static_cast<int>(data->type)
				<< DEFAULT << std::endl;
	}
}

void Epoll::clearEventMap(void)
{
	for (auto it = this->fdEventMap.begin();
			it != fdEventMap.end(); ++it)
	{
		delete it->second;
	}
	this->fdEventMap.clear();
}
