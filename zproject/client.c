#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> // для sockaddr_in, inet_pton
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>

#define MAX_BYTES 4096

typedef struct t_data
{
	int server_fd;
	pthread_t p1;
	pthread_t p2;
} s_data;

int connect_server(const char *ip, int port)
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		perror("socket");
		exit(1);
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0)
	{
		perror("inet_pton");
		close(server_fd);
		exit(1);
	}

	if (connect(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		perror("connect");
		close(server_fd);
		exit(1);
	}
	return server_fd;
}

void *read_data(void *arg)
{
	char buffer[MAX_BYTES];
	s_data *data = (s_data *)arg;
	ssize_t bytes;

	while (1)
	{
		bytes = read(data->server_fd, buffer, sizeof(buffer));
		if (bytes == 0)
		{
			printf("Server dissconnected\n");
			break;
		}
		if (bytes < 0)
		{
			perror("read");
			break;
		}
		if (write(STDOUT_FILENO, buffer, bytes) == -1)
		{
			perror("[read_data] write");
			break;
		}
	}
	pthread_cancel(data->p2); // to stop the write_data thread
	return NULL;
}

void *write_data(void *arg)
{
	char buffer[MAX_BYTES];
	s_data *data = (s_data *)arg;
	ssize_t bytes;

	printf("Enter some data:\n");
	while (1)
	{
		bytes = read(STDIN_FILENO, buffer, sizeof(buffer));
		if (bytes <= 0)
			break;
		if (write(data->server_fd, buffer, bytes) == -1)
		{
			perror("[write_data] write");
			break;
		}
	}
	return NULL;
}

int main(int argc, char **argv)
{
	int rc;
	s_data data;
	if (argc == 3)
	{
		char *ip = argv[1];
		int port = atoi(argv[2]);
		data.server_fd = connect_server(ip, port);
	}
	else
	{
		char *ip = "127.0.0.1";
		int port = 8080;
		data.server_fd = connect_server(ip, port);
	}
	rc = pthread_create(&data.p1, NULL, read_data, &data);
	if (rc != 0)
	{
		fprintf(stderr, "pthread_create: %s\n", strerror(rc));
		close(data.server_fd);
		exit(1);
	}
	rc = pthread_create(&data.p2, NULL, write_data, &data);
	if (rc != 0)
	{
		fprintf(stderr, "pthread_create: %s\n", strerror(rc));
		close(data.server_fd);
		exit(1);
	}

	rc = pthread_join(data.p1, NULL);
	if (rc != 0)
	{
		fprintf(stderr, "pthread_join: %s\n", strerror(rc));
		close(data.server_fd);
		exit(1);
	}
	rc = pthread_join(data.p2, NULL);
	if (rc != 0)
	{
		fprintf(stderr, "pthread_join: %s\n", strerror(rc));
		close(data.server_fd);
		exit(1);
	}
	close(data.server_fd);
	return 0;
}
