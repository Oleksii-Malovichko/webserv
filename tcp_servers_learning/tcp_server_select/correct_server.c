#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_BYTES 4096
#define MAX_CLIENTS 2

int create_socket() // socket
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

void handle_client(int *client_fd, char *write_buf, size_t *write_len) // read
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
	{
		*write_len = snprintf(write_buf, MAX_BYTES, "You wrote: %.*s", (int)bytes, buffer); // save data for the future echo-reply
		if (*write_len >= MAX_BYTES)
			*write_len = MAX_BYTES - 1;
		printf("Client(%d): %.*s", *client_fd, (int)bytes, buffer);
	}
}

int add_client(int server_fd, int *clients_fds) // accept
{
	int i = 0;
	int client_fd = accept(server_fd, NULL, NULL);
	if (client_fd == -1)
	{
		if (errno != EWOULDBLOCK && errno != EINTR && errno != EAGAIN)
			perror("accept");
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
		return 0;
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
	char *bf = "The server is full, connection rejected\n";
	fprintf(stderr, "%s", bf);
	ssize_t n = write(client_fd, bf, strlen(bf));
	if (n == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
		perror("write");
	close(client_fd);
	return 1; // the accept worked anyway sucesfully
}

int main()
{
	int server_fd = create_socket();
	int clients_fds[MAX_CLIENTS];
	char write_buf[MAX_CLIENTS][MAX_BYTES]; // буфер для ответа (где хранятся дескрипторы клиентов и их сообщения)
	size_t clients_len[MAX_CLIENTS]; // сколько байт всего буфере
	size_t clients_sent[MAX_CLIENTS]; // сколько уже отправлено
	int i = 0;
	int j = 0;

	memset(clients_fds, -1, sizeof(clients_fds));
	memset(clients_len, 0, sizeof(clients_len));
	memset(clients_sent, 0, sizeof(clients_sent));
	while (1)
	{
		fd_set readfd, writefd;
		FD_ZERO(&readfd);
		FD_ZERO(&writefd);
		FD_SET(server_fd, &readfd);
		// FD_SET(server_fd, &writefd);
		int maxfd = server_fd;

		i = 0;
		while (i < MAX_CLIENTS) // заполнить fd_set
		{
			if (clients_fds[i] != -1)
			{
				FD_SET(clients_fds[i], &readfd);
				FD_SET(clients_fds[i], &writefd);
				if (clients_fds[i] > maxfd)
					maxfd = clients_fds[i];
			}
			i++;
		}
		
		int activity = select(maxfd + 1, &readfd, &writefd, NULL, NULL); // 1250 == max clients the select can handle; epoll == infite
		if (activity < 0)
		{
			if (errno == EINTR) // был прерван сигналом - продолжаем работу без ошибки
				continue;
			perror("select");
			continue;
		}

		if (FD_ISSET(server_fd, &readfd)) // accept
		{
			while (add_client(server_fd, clients_fds)) {}; // лучше в цикле, так успеем больше обработать
		}
		j = 0;
		while (j < MAX_CLIENTS)
		{
			if (FD_ISSET(clients_fds[j], &writefd) && clients_sent[j] < clients_len[j])
			{
				ssize_t n = write(clients_fds[j], write_buf[j] + clients_sent[j], clients_len[j] - clients_sent[j]);
				if (n > 0)
					clients_sent[j] += n;
				else if (n == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
				{
					perror("write");
					close(clients_fds[j]);
					clients_fds[j] = -1;
					clients_len[j] = clients_sent[j] = 0;
				}
				if (clients_sent[j] == clients_len[j])
					clients_len[j] = clients_sent[j] = 0; // все отправлено
			}
			j++;
		}
		j = 0;
		while (j < MAX_CLIENTS)
		{
			if (clients_fds[j] != -1 && FD_ISSET(clients_fds[j], &readfd)) // read from client and save the data in write_buf
				handle_client(&clients_fds[j], write_buf[j], &clients_len[j]);
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
