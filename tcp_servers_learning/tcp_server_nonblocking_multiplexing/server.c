#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_CLIENTS 2

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
	// делаем сокет non-blocking (для accept)
	int flags = fcntl(server_fd, F_GETFL, 0); // получить флаги (может быть как nonblock так и block)
	// тут мы исп fcntl выше, чтобы не потерять пред флаги, которые установила ОС
	// так как ф-я ниже сбрасывает ВСЕ флаги и устанавливает все по новой
	// образно это выглядит так: flags += O_NONBLOCK; // образно
	fcntl(server_fd, F_SETFL, flags | O_NONBLOCK); // а тут мы добавляем флаг nonblock (чтобы гарантировать что сервер не зависнет на одном клиенте)

	struct sockaddr_in addr;
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
	char buffer[1024];
	ssize_t bytes = -1;

	bytes = read(*client_fd, buffer, sizeof(buffer));
	if (bytes < 0) // error (close conn)
	{
		if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) // read didn't get any data (return)
			return ;
		perror("read");
		close(*client_fd);
		*client_fd = -1;
	}
	else if (bytes == 0) // OK (close conn)
	{
		printf("Client(%d) closed connection\n", *client_fd);
		close(*client_fd);
		*client_fd = -1;
	}
	else // get data
	{
		printf("Client(%d): %.*s\n", *client_fd, (int)bytes, buffer);
	}
}

int main()
{
	int server_fd = create_socket();
	int client_fd = -1;
	int i;
	int added;
	int j;
	int clients_fds[MAX_CLIENTS];

	memset(clients_fds, -1, sizeof(clients_fds));
	while (1)
	{
		client_fd = accept(server_fd, NULL, NULL);
		if (client_fd == -1)
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EAGAIN)
				perror("accept");
		}
		else
		{
			int flags = fcntl(client_fd, F_GETFL, 0); // сохранить флаги
			fcntl(client_fd, F_SETFL, flags | O_NONBLOCK); // добавить неблокирующий флаг
			i = 0;
			added = 0;
			while (i < MAX_CLIENTS) // добавить в массив
			{
				if (clients_fds[i] == -1)
				{
					clients_fds[i] = client_fd;
					printf("Added new client(%d)\n", client_fd);
					added = 1;
					break; // тут можно использовать флаг, если места в массиве уже нет, то отправить одно сообщение клиенту: попробуйте подключится позже
				}
				i++;
			}
			if (!added)
			{
				char *buffer = "Server is full, request reqected\n";
				if (write(client_fd, buffer, strlen(buffer)) == -1)
					perror("write");
				printf(buffer);
				close(client_fd);
			}
		}
		j = 0;
		while (j < MAX_CLIENTS)
		{
			if (clients_fds[j] != -1)
				handle_client(&clients_fds[j]);
			j++;
		}
		usleep(1000); // pause 1 ms
	}
	i = 0;
	while (i < MAX_CLIENTS)
	{
		if (clients_fds[i] != -1)
			close(clients_fds[i]);
		i++;
	}
	close(server_fd);
	return 0;
}