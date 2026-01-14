#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>

int create_socket()
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		perror("socket");
		exit(1);
	}

	int flags = fcntl(server_fd, F_GETFL, 0);
	if (flags == -1)
	{
		perror("fcntl(F_GETFL)");
		close(server_fd);
		exit(1);
	}
	if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		perror("fcntl(F_SETFL)");
		close(server_fd);
		exit(1);
	}

	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		perror("setsockopt");
		close(server_fd);
		exit(1);
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8080);
	addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		perror("bind");
		close(server_fd);
		exit(1);
	}

	if (listen(server_fd, 5) == -1)
	{
		perror("listen");
		close(server_fd);
		exit(1);
	}
	return server_fd;
}

void handle_client(int client_fd, int epfd)
{
	char buffer[4096];

	ssize_t bytes = read(client_fd, buffer, sizeof(buffer));
	if (bytes < 0)
	{
		if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR)
			return ;
		perror("read");
	}
	else if (bytes == 0)
		printf("Client(%d) closed connection\n", client_fd);
	else
	{
		printf("Client(%d): %.*s\n", client_fd, (int)bytes, buffer);
		return ;
	}
	if (epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, NULL))
		perror("epoll_ctl DEL");
	close(client_fd);
}

int accept_client(int server_fd, int epfd)
{
	int client_fd = accept(server_fd, NULL, NULL);
	if (client_fd == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return 0;
		if (errno == EINTR)
			return 1;
		perror("accept");
		return 0;
	}
	int flags = fcntl(client_fd, F_GETFL, 0);
	fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

	struct epoll_event cev;
	cev.data.fd = client_fd;
	cev.events = EPOLLIN;

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &cev) == -1)
	{
		perror("epoll_ctl ADD");
		close(client_fd);
		return 0;
	}
	printf("Added new client(%d)\n", client_fd);
	return 1;
}

#define MAX_EVENTS 16

int main()
{
	int epfd = epoll_create1(0);
	if (epfd == -1)
	{
		perror("epoll");
		exit(1);
	}
	int server_fd = create_socket();
	int i;

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = server_fd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev) == -1) // системный вызов, что мы хотим просматиравать событие (EPOLLIN) на нашем server_fd
	{
		perror("epoll_ctl ADD");
		close(server_fd);
		exit(1);
	}
	struct epoll_event events[MAX_EVENTS]; // будующий массив, где мы будем хранить к-во измененных дескрипторов за раз

	while (1)
	{
		int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
		if (n == -1)
		{
			if (errno == EINTR)
				continue;
			perror("epoll_wait");
			break;
		}
		i = 0;
		while (i < n)
		{
			int fd = events[i].data.fd;
			if (fd == server_fd) // значит чтото произошло с server_fd -> accept
			{
				while (accept_client(server_fd, epfd)) {};
			}
			else // значит новый клиент
				handle_client(fd, epfd);
			i++;
		}
	}
	close(server_fd);
	close(epfd);
	return 0;
}