#include "Server.hpp"

bool Server::running = true;

Server::Server(const std::string &configFile)
{
	// вместо конфиг файла
	
	(void)configFile;
	ServerConfig s1;
	s1.setPort(8080);
	configs.push_back(s1);

	// создаем listening sockets и добавляем в epoll
	for (size_t i = 0; i < configs.size(); i++)
	{
		ListeningSocket sock(configs[i].getIP(), configs[i].getPort());
		epoll.addListeningSocket(std::move(sock));
		std::cout << "Listening on port " << configs[i].getPort() << std::endl;
	}
}

void Server::sigintHandler(int sig)
{
	(void)sig;
	running = 0;
}

void Server::run()
{
	std::signal(SIGINT, sigintHandler);
	while (running)
	{
		// epoll делает всю работу
		epoll.handleEvents(1000); // дефолтное значение таймаута, пока нет клиентов
		
		// Сервер решает, что делать с данными
		const auto &clients = epoll.getClients();

		for (auto it = clients.begin(); it != clients.end(); it++)
		{
			Client &client = const_cast<Client&>(it->second);
			if (client.getState() != Client::State::READING)
				continue;
			if (!client.getReadBuffer().empty())
				handleClient(client);
		}
	}
	shutdownServer();
}

void Server::handleClient(Client &client)
{
	const std::string &data = client.getReadBuffer();
	if (data == "12345" || data == "12345\n")
	{
		std::cout << "Client stopped server" << std::endl;
		running = false;
		return ;
	}

	const std::string &back_msg = "Server: " + data;

	client.appendToWriteBuffer(back_msg);
	client.clearReadBuffer();
	client.setState(Client::State::WRITING);
	epoll.updateClientEvents(client);
}

void Server::shutdownServer()
{
	std::cout << "Shutting down server..." << std::endl;

	auto &clients = epoll.getClients();
	for (auto &pair : clients)
		pair.second.close();
}

void Server::handleCGI(Client &client)
{
	(void)client;
}
