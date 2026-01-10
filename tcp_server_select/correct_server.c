#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>

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
	if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
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

void handle_client(int *client_fd) // here would be better to use gnl for reading
{
	char buffer[4096];

	ssize_t bytes = read(*client_fd, &buffer, sizeof(buffer));
	if (bytes < 0)
	{
		if (errno == EWOULDBLOCK || errno == EINTR || errno == EAGAIN)
			return ; // просто пробуем позже
		perror("read");
		close(*client_fd);
		*client_fd = -1;
		return ;
	}
	else if (bytes == 0)
	{
		printf("Client(%d) closed connection\n", *client_fd);
		close(*client_fd);
		*client_fd = -1;
		return ;
	}
	else
		printf("Client(%d): %.*s", *client_fd, (int)bytes, buffer);
}

#define MAX_CLIENTS 2

int add_client(int server_fd, int *clients_fds)
{
	int i = 0;
	int client_fd = accept(server_fd, NULL, NULL);
	if (client_fd == -1)
	{
		if (errno != EWOULDBLOCK && errno != EINTR && errno != EAGAIN)
			perror("accept");
		if (errno == EINTR)
			return 1; // accept был прерван, пробуем еще раз
		return 0;
	}
	int flags = fcntl(client_fd, F_GETFL, 0);
	if (flags == -1)
	{
		perror("fcntl(F_GETFL)");
		close(client_fd);
		return 0;
	}
	if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		perror("fcntl(F_SETFL)");
		close(client_fd);
		return 1;
	}
	while (i < MAX_CLIENTS)
	{
		if (clients_fds[i] == -1)
		{
			clients_fds[i] = client_fd;
			printf("Added client(%d)\n", client_fd);
			return 1;
		}
		i++;
	}
	printf("The server is full, connection rejected\n");
	close(client_fd);
	return 1; // the accept worked anyway sucesfully
}

int main()
{
	int server_fd = create_socket();
	int clients_fds[MAX_CLIENTS];
	int i = 0;
	int j = 0;

	memset(clients_fds, -1, sizeof(clients_fds));
	while (1)
	{
		fd_set readfd;
		FD_ZERO(&readfd);
		FD_SET(server_fd, &readfd);
		int maxfd = server_fd;

		i = 0;
		while (i < MAX_CLIENTS) // заполнить fd_set
		{
			if (clients_fds[i] != -1)
			{
				FD_SET(clients_fds[i], &readfd);
				if (clients_fds[i] > maxfd)
					maxfd = clients_fds[i];
			}
			i++;
		}
		
		int activity = select(maxfd + 1, &readfd, NULL, NULL, NULL);
		if (activity < 0)
		{
			if (errno == EINTR) // был прерван сигналом - продолжаем работу без ошибки
				continue;
			perror("select");
			continue;
		}

		if (FD_ISSET(server_fd, &readfd)) // есть ли событие для сервера (для accept)
		{
			while (add_client(server_fd, &clients_fds)) {}; // лучше в цикле, так успеем больше обработать
		}
		j = 0;
		while (j < MAX_CLIENTS)
		{
			if (clients_fds[j] != -1 && FD_ISSET(clients_fds[j], &readfd))
				handle_client(&clients_fds[j]);
			j++;
		}
	}
	i = 0;
	while (i < MAX_CLIENTS)
	{
		if (clients_fds[i] != -1)
		{
			close(clients_fds[i]);
		}
		i++;
	}
	close(server_fd);
	return 0;
}
