#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

// void create_socket()
// {
// 	int server_fd = socket(AF_INET, SOCK_STREAM, 0); // точка приема подключений (слушает новые подключения)
// 	if (server_fd == -1)
// 	{
// 		perror("socket");
// 		exit(1);
// 	}
// 	// AF_INET - IPv4
// 	// SOCK_STREAM - TCP
// 	// server_fd = будущий слушащий сокет

// 	int opt = 1;
// 	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) // позволяет повторно исп адрес, работает только с server_fd
// 	{
// 		perror("setsockopt");
// 		exit(1);
// 	}

// 	// привязка сокета к ip и порту
// 	struct sockaddr_in addr;
// 	memset(&addr, 0, sizeof(addr)); // обнуление структуры для безопасности от мусора
// 	addr.sin_family = AF_INET;
// 	addr.sin_port = htons(8080);
// 	addr.sin_addr.s_addr = INADDR_ANY;

// 	if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) // теперь порт принадлежит этому fd, bind не блокирует, fd остается тем же
// 	{
// 		perror("bind");
// 		close(server_fd);
// 		exit(1);
// 	}

// 	if (listen(server_fd, 5) == -1) // перевод сокета в режим ожидания соединения (5 - размер очереди клиентов)
// 	{
// 		perror("listen");
// 		close(server_fd);
// 		exit(1);
// 	}

// 	int client_fd;
// 	client_fd = accept(server_fd, NULL, NULL); // если клиентов нет - блокируется. server_fd остается жить. client_fd для read/write
// 	if (client_fd == -1)
// 	{
// 		perror("accept");
// 		close(server_fd);
// 		exit(1);
// 	}

// 	char buffer[1024];
// 	ssize_t bytes;
// 	bytes = read(client_fd, buffer, sizeof(buffer) - 1); // блокируется пока клиент не отправит данные. bytes == 0 клиент закрыл соед,
// 	if (bytes > 0)
// 	{
// 		buffer[bytes] = '\0';
// 		printf("The gotten data: %s\n", buffer);
// 	}
// 	else if (bytes == 0)
// 	{
// 		printf("Client closed connection\n");
// 	}
// 	else
// 	{
// 		perror("read");
// 	}
// 	close(client_fd);
// 	close(server_fd);
// }

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
	char buffer[1024];
	ssize_t bytes = read(*client_fd, buffer, sizeof(buffer));
	/* 
	Именно из-за этой ф-ии сервер является blocking, так как он зависает на read, и когда я пытаюсь подключится к серверу с другого клиента, то у меня 
	это получается, но я не могу ничего отправить серверу, так как его read обрабатывает первого клиента. Также нужно добавить ф-ю fcntl, которая сделает
	сам сокет non-blocking
	*/
	if (bytes < 0)
	{
		perror("read");
		close(*client_fd);
		*client_fd = -1;
	}
	else if (bytes == 0)
	{
		printf("Client: %d closed connection\n", *client_fd);
		close(*client_fd);
		*client_fd = -1;
	}
	else
	{
		printf("Client(%d): %.*s\n", *client_fd, (int)bytes, buffer);
	}
}

int main()
{
	int server_fd = create_socket();
	int client_fd = -1;

	while (1)
	{
		if (client_fd == -1)
		{
			client_fd = accept(server_fd, NULL, NULL);
			if (client_fd == -1)
			{
				perror("accept");
				continue; // сервер не должен ломаться из-за одного клиента!!!
			}
			printf("Client %d connected\n", client_fd);
		}
		handle_client(&client_fd);
	}
	close(client_fd);
	close(server_fd);
	return 0;
}
