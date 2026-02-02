#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
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
	addr.sin_port = htons(8080);
	addr.sin_family = AF_INET;
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
	ssize_t bytes;

	bytes = read(client_fd, buffer, sizeof(buffer));
	if (bytes == 0)
		printf("Client(%d) closed connection\n", client_fd);
	else if (bytes < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			return ;
		perror("read");
	}
	else
	{
		printf("Client(%d): %.*s\n", client_fd, (int)bytes, buffer);
		return ;
	}
	epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, NULL); // удаление fd из epoll (системный вызов)
	close(client_fd);
}

#define MAX_EVENTS 16

int main()
{
	int server_fd = create_socket();
	int i;
	int epfd = epoll_create1(0); // дескриптор epoll-объекта (ядро хранит там список fd, интересующие события, очередь готовых событий. Аналог FILE *)
	if (epfd == -1)
	{
		perror("epoll_create1");
		exit(1);
	}

	struct epoll_event ev; // эта структура играет роль конверта, чтобы затем ее как аргумент передать epoll_ctl
	// ключевая структура epoll (создаем описание события, которое хотим отслеживать)
	/* 
	struct epoll_event
	{
		uint32_t events; // какие события интересуют
		epoll_data_t data; // пользовательские данные
	} */
	ev.events = EPOLLIN; // говорим ядру: меня интересует, когда server_fd станет готов к чтению (пришел новый клиент - можно accept)
	ev.data.fd = server_fd;
	/* 
	data - это мои данные, ядро их не интерпретирует
	я кладу туда: fd (как тут), или указательна struct client, или ID соединение. Позже ядро вернет мне это обратно */

	epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev); // основной системный вызов, где мы просим ядро отслеживать определенный дескриптор по опред событию
	// команда ядру: добавь server_fd в epoll и следи за EPOLLIN
	/* 
	теперь epoll помнит этот fd, следит за ним и будет уведомлять */
	// где epoll говорит о событиях?
	struct epoll_event events[MAX_EVENTS]; // тут. Это буфер, который я выделяю, куда ядро запишет список произошедших событий

	while (1)
	{
		int n = epoll_wait(epfd, events, MAX_EVENTS, -1); // блокируется пока не произойдет событие или не придет сигнал
		/* 
		n - сколько fd изменили состояние
		epfd - какой epoll
		events - куда писать события
		MAX_EVENTS - максимум за раз
		-1 - ждать бесконечно
		 */
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
			int fd = events[i].data.fd; // это тот самый data, который я положил при epoll_ctl (ядро говорит: вот fd, по которому чтото произошло)
			/* 
			А инфа, что именно произошло - events[i].events
			например:
			EPOLLIN - можно читать
			EPOLLERR - ошибка
			EPOLLHUP - повесили трубку (клиент резко отключился) */
			if (fd == server_fd)
			{
				// accept to all pending connections
				while (1)
				{
					int client_fd = accept(server_fd, NULL, NULL);
					if (client_fd == -1)
					{
						if (errno == EAGAIN || errno == EWOULDBLOCK)
							break;
						perror("accept");
						break;
					}
					int flags = fcntl(client_fd, F_GETFL, 0);
					fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

					// это другое описание для другого fd. (server_fd -> accept; client_fd -> read)
					/* 
					каждый fd регистрируется отдельно
					со своими событиями
					со своими user-data */
					struct epoll_event cev;
					cev.events = EPOLLIN;
					cev.data.fd = client_fd;

					epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &cev); // добавить client_fd в ядро
					printf("Added client(%d)\n", client_fd);
				}
			}
			else
				handle_client(fd, epfd);
			i++;
		}
	}
	close(server_fd);
	close(epfd);
	return 0;
}

/* 
В данном коде epoll используется как ядровый диспетчер готовности файловых дескрипторов.
Пользовательское пространство (мой код) не хранит (пока что) список соединений, а полностью полагается на ядро
для отслеживания их состояния, обрабатывая только уже готовые события

Ключевая мысль:
epoll_event - это НЕ событие, а контракт между мной и ядром.
Я описываю, что меня интересует
Ядро сообщает, что произошло
через ту же структуру*/