#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int create_socket()
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		perror("socket");
		exit(1);
	}

	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		perror("setsockopt");
		close(server_fd);
		exit(1);
	}

	int flags = fcntl(server_fd, F_GETFL, 0);
	fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

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

void handle_client(int *client_fd)
{
	char buffer[4096];

	ssize_t bytes = read(*client_fd, buffer, sizeof(buffer));
	if (bytes < 0)
	{
		if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
			return ;
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
		printf("Client(%d): %.*s\n", *client_fd, (int)bytes, buffer);
}

#define MAX_CLIENTS 2

int main()
{
	int server_fd = create_socket();
	int clients_fds[MAX_CLIENTS];
	int client_fd = -1;
	int i;
	int j;
	int added;

	memset(clients_fds, -1, sizeof(clients_fds));
	while (1)
	{
		client_fd = accept(server_fd, NULL, NULL);
		if (client_fd == -1)
		{
			if (errno != EINTR && errno != EWOULDBLOCK && errno != EAGAIN)
				perror("accept");
		}
		else
		{
			int flags = fcntl(client_fd, F_GETFL, 0);
			fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
			i = 0;
			added = 0;
			while (i < MAX_CLIENTS)
			{
				if (clients_fds[i] == -1)
				{
					clients_fds[i] = client_fd;
					printf("Added new client(%d)\n", client_fd);
					added = 1;
					break;
				}	
				i++;
			}
			if (!added)
			{
				printf("The server is full, the connection is rejected\n");
				close(client_fd);
				client_fd = -1;
			}
		}
		j = 0;
		while (j < MAX_CLIENTS)
		{
			if (clients_fds[j] != -1)
			{
				handle_client(&clients_fds[j]);
			}
			j++;
		}
		usleep(1000);
	}
	i = 0;
	while (i < MAX_CLIENTS)
	{
		if (clients_fds[i] != -1)
			close(clients_fds[i]);
		i++;
	}
	close(server_fd);
}