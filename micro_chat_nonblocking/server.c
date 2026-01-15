#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_CLIENTS 2

void error(char *error, int fd)
{
	perror(error);
	close(fd);
	exit(1);
}

void close_clients(int *clients_fds)
{
	int i = 0;
	while (i < MAX_CLIENTS)
	{
		if (clients_fds[i] != -1)
			close(clients_fds[i]);
		i++;
	}
}

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
		error("fcntl F_GETFL", server_fd);
	if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) == -1)
		error("fcntl F_SETFL", server_fd);

	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		error("setsockopt", server_fd);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8080);
	addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
		error("bind", server_fd);

	if (listen(server_fd, 5) == -1)
		error("bind", server_fd);

	return server_fd;
}

void read_client(int *client_fd)
{
	char buffer[4096];
	ssize_t bytes;

	bytes = read(*client_fd, buffer, sizeof(buffer));
	if (bytes < 0)
	{
		if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
			return ;
		perror("read");
	}
	else if (bytes == 0)
	{
		printf("Client(%d) closed connection\n", *client_fd);
	}
	else
	{
		printf("Client(%d): %.*s\n", *client_fd, (int)bytes, buffer);
		return ;
	}
	close(*client_fd);
	*client_fd = -1;
}

char *get_stdin_data()
{
	char buffer[4096];

	ssize_t bytes = read(0, buffer, sizeof(buffer));
	if (bytes < 0)
	{
		if (errno == EWOULDBLOCK || errno == EINTR)
			return NULL;
		perror("read STDIN");
		return NULL;
	}
	else if (bytes == 0)
		return NULL;
	char *res = strndup(buffer, bytes);
	return res;
}

int add_client(int server_fd, int *clients_fds)
{
	int added = 0;
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
	if (flags == -1)
	{
		perror("fcntl F_GETFL");
		return 1;
	}
	if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		perror("fcntl F_SETFL");
		return 1;
	}
	int i = 0;
	while (i < MAX_CLIENTS)
	{
		if (clients_fds[i] == -1)
		{
			clients_fds[i] = client_fd;
			added = 1;
			printf("Added client(%d)\n", client_fd);
			break ;
		}
		i++;
	}
	if (!added)
	{
		char *bf = "The server is full. The connection rejected\n";
		if (write(client_fd, bf, strlen(bf)) == -1)
		{
			perror("write");
		}
		printf("%s", bf);
		close(client_fd);
		return 1;
	}
	return 1;
}

int main()
{
	int server_fd = create_socket();
	int clients_fds[MAX_CLIENTS];
	int i = 0;
	char *write_bf = NULL;

	memset(clients_fds, -1, sizeof(clients_fds));
	while (1)
	{
		// сделать stdin non-blocking
		int flags = fcntl(0, F_GETFL, 0);
		if (flags == -1)
		{
			close_clients(clients_fds);
			error("fcntl F_GETFL", server_fd);
		}
		if (fcntl(0, F_SETFL, flags | O_NONBLOCK))
		{
			close_clients(clients_fds);
			error("fcntl F_SETFL", server_fd);
		}
		write_bf = get_stdin_data();
		if (write_bf)
		{
			i = 0;
			while (i < MAX_CLIENTS)
			{
				if (write(clients_fds[i], write_bf, strlen(write_bf)) == -1)
				{
					perror("write");
					break;
				}
				i++;
			}
		}
		free(write_bf);
		write_bf = NULL;
		while (add_client(server_fd, clients_fds)) {};
		i = 0;
		while (i < MAX_CLIENTS)
		{
			if (clients_fds[i] != -1)
				read_client(&clients_fds[i]);
			i++;
		}
		usleep(1000);
	}
	close_clients(clients_fds);
	close(server_fd);
	return 0;
}