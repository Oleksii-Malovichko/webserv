#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <unordered_map>
#include <chrono>
#include <iostream>

struct Client
{
	int fd;
	std::chrono::steady_clock::time_point last_activity;
};

int create_listen_socket(int port)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("socket");
		exit(1);
	}

	int flags = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, flags | O_NONBLOCK);

	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if (bind(sock, (sockaddr *)&addr, sizeof(addr)) < 0)
	{
		perror("bind");
		exit(1);
	}
	if (listen(sock, 10) < 0)
	{
		perror("listen");
		exit(1);
	}
	return sock;
}

int main()
{
	int listen_sock = create_listen_socket(8080);

	int epfd = epoll_create1(0);
	if (epfd < 0)
	{
		perror("epoll_create1");
		exit(1);
	}

	epoll_event ev{};
	ev.events = EPOLLIN;
	ev.data.fd = listen_sock;
	epoll_ctl(epfd, EPOLL_CTL_ADD, listen_sock, &ev);

	std::unordered_map<int, Client> clients;
	const int MAX_EVENTS = 10;
	const int TIMEOUT_MS = 1000; // check timeout on time per sec
	const int CLIENT_TIMEOUT_SEC = 30; // close a client, if no actions > 30 sec

	epoll_event events[MAX_EVENTS];

	while (true)
	{
		int n = epoll_wait(epfd, events, MAX_EVENTS, TIMEOUT_MS);

		auto now = std::chrono::steady_clock::now();

		// check the timeout
		for (auto it = clients.begin(); it != clients.end();)
		{
			auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.last_activity);
			if (duration.count() > CLIENT_TIMEOUT_SEC)
			{
				std::cout << "Close idle client fd=" << it->first << " after 30 seconds of non-action" << std::endl;
				close(it->first);
				epoll_ctl(epfd, EPOLL_CTL_DEL, it->first, nullptr);
				it = clients.erase(it);
			}
			else
				it++;
		}

		for (int i = 0; i < n; i++)
		{
			int fd = events[i].data.fd;

			if (fd == listen_sock)
			{
				sockaddr_in client_addr{};
				socklen_t len = sizeof(client_addr);
				int client_fd = accept(listen_sock, (sockaddr*)&client_addr, &len);
				if (client_fd < 0)
					continue;
				int flags = fcntl(client_fd, F_GETFL, 0);
				fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

				epoll_event client_ev{};
				client_ev.events = EPOLLIN;
				client_ev.data.fd = client_fd;
				epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &client_ev);

				clients[client_fd] = {client_fd, now};
				std::cout << "New client fd=" << client_fd << std::endl;
			}
			else
			{
				// read / echo
				char buf[1024];
				int nread = read(fd, buf, sizeof(buf));
				if (nread <= 0)
				{
					std::cout << "Closing client fd=" << fd << std::endl;
					close(fd);
					epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
					clients.erase(fd);
				}
				else
				{
					write(fd, buf, nread); // just echo
					clients[fd].last_activity = now;
				}
			}
		}
	}
	close(listen_sock);
	close(epfd);
	return 0;
}