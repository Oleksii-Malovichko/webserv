#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int create_socket(void)
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		perror("socket");
		exit(1);
	}

	int flags = fcntl(server_fd, F_GETFL, 0);
	fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		perror("setsocketopt");
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

void handle_client(int *client_fd)
{
	char buffer[4096];

	ssize_t bytes = read(*client_fd, buffer, sizeof(buffer));
	if (bytes < 0)
	{
		if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
			return ;
		perror("read");
	}
	else if (bytes == 0)
		printf("Client(%d) finished connection\n", *client_fd);
	else
	{
		printf("Client(%d): %.*s\n", *client_fd, (int)bytes, buffer);
		return ;
	}
	close(*client_fd);
	*client_fd = -1;
}

#define MAX_CLIENTS 2

int main()
{
	int server_fd;
	int i;
	int j;
	int added;
	int clients_fds[MAX_CLIENTS];
	int client_fd;

	memset(clients_fds, -1, sizeof(clients_fds));
	server_fd = create_socket();
	while (1)
	{
		fd_set readfds; // создаем набор дескрипторов, которые мы хотим отслеживать на готовность к чтению
		// fd_set это структура для select, которая хранит множество сокетов
		FD_ZERO(&readfds); // очищаем набор, чтобы там не было мусора
		FD_SET(server_fd, &readfds); // добавляем наш серверный сокет в набор
		int maxfd = server_fd; // select требует максимальный дескриптор + 1 (nfds). мы пока знаем только серверный сокет
		i = 0;
		// тут я подготавливаю список сокетов, который затем буду проверять на готовность
		while (i < MAX_CLIENTS)
		{
			if (clients_fds[i] != -1)
			{
				FD_SET(clients_fds[i], &readfds); // добавляем каждого клиента в набор для select.
				// это значит: "сообщи мне, когда на этом клиентском сокете есть данные для чтения"
				if (clients_fds[i] > maxfd) // обновляем максимальный дескрпитор, потому что select требует его
					maxfd = clients_fds[i];
			}
			i++;
		}
		// итерация на готовность дескрипторов происходит тут
		int activity = select(maxfd + 1, &readfds, NULL, NULL, NULL); // activity - к-во сокетов которые готовы
		// nfds = maxfd + 1 - максимальный дескриптор + 1.  select просматривает дескрипторы от 0 до nfds-1
		// &readfds - набор дескрипторов, за которыми нужно следить для чтения
		// NULL - второй аргумент для записи (writefds), пока не исп
		// NULL - третий аргумент для исключительных событий (exceptfds), пока нет
		// NULL - таймаут (struct timeval *timeout). NULL = ждать бесконечно, пока не появится событие 
		if (activity < 0) // ошибка, ждем клиентов
		{
			perror("select");
			continue;
		}
		// тут мы проверяем, пришел ли новый клиент
		if (FD_ISSET(server_fd, &readfds)) // проверяет есть ли событие на сокете fd. В нашем случае, если серверный сокет готов,
		{									// значит клиент пытается подключиться и можно безопасно вызвать accept()
			client_fd = accept(server_fd, NULL, NULL); // NULL не нужно сохранять адрес клиента (обычно struct sockaddr *); NULL размер структуры адреса
			if (client_fd == -1)
			{
				if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
					perror("accept");
			}
			else
			{
				int flags = fcntl(client_fd, F_GETFL, 0);
				fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
				added = 0;
				i = 0;
				while (i < MAX_CLIENTS)
				{
					if (clients_fds[i] == -1)
					{
						clients_fds[i] = client_fd;
						added = 1;
						printf("Added new client(%d)\n", client_fd);
						break;
					}
					i++;
				}
				if (!added)
				{
					printf("The server is full, connection rejected\n");
					close(client_fd);
				}
			}
		}
		j = 0;
		// а тут мы уже проверяем готов ли клиент на чтение, ДРУГОЙ СОКЕТ
		while (j < MAX_CLIENTS)
		{
			if (clients_fds[j] != -1 && FD_ISSET(clients_fds[j], &readfds)) // FD_ISSET говорит: есть ли анные для read(), если да вызваем handle_client
				handle_client(&clients_fds[j]);
			j++;
		}
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

/* 
fd_set — просто структура, куда ты добавляешь дескрипторы, за которыми хочешь следить.
	FD_ZERO — очищает её.
	FD_SET — добавляет дескриптор.
maxfd — нужен select, чтобы знать, до какого дескриптора проверять.
	select просматривает от 0 до maxfd.
	Без него пришлось бы проверять все дескрипторы до огромного числа, даже если их нет.
select — главный игрок:
	проверяет каждый дескриптор на готовность (read/write/except).
	обновляет fd_set, оставляя только готовые.
	возвращает число готовых дескрипторов (activity).
FD_ISSET — просто спрашивает: «а остался ли этот дескриптор в fd_set после select?»
	если да → можно безопасно вызвать accept или read.
Короче говоря: select делает всю работу, а макросы просто помогают подготовить набор и проверить результат. */

/* 
| Функция                                                                                           | Аргументы                    | Что делает                                                      |
| ------------------------------------------------------------------------------------------------- | ---------------------------- | --------------------------------------------------------------- |
| `FD_ZERO(fd_set *set)`                                                                            | set — указатель на fd_set    | Очищает набор дескрипторов                                      |
| `FD_SET(int fd, fd_set *set)`                                                                     | fd — дескриптор, set — набор | Добавляет дескриптор в набор                                    |
| `FD_CLR(int fd, fd_set *set)`                                                                     | fd, set                      | Убирает дескриптор из набора                                    |
| `FD_ISSET(int fd, fd_set *set)`                                                                   | fd, set                      | Проверяет, есть ли дескриптор в наборе (т.е. событие произошло) |
| `select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)` | см. выше                     | Блокируется до события на одном из дескрипторов                 |
 */


/* 
Доки для штутгарта (annerkennung) +
lebenslauf + 
anschreiben +

*/