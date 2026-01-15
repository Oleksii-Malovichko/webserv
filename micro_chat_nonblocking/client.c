#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>

int connect_server()
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
	addr.sin_port = htons(8080);
	if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0)
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
	return (server_fd);
}

void *get_msg(void *arg)
{
	int server_fd = *(int *)arg;
	char buffer[4096];

	ssize_t bytes;
	while ((bytes = read(server_fd, buffer, sizeof(buffer))) > 0)
	{
		if (write(1, buffer, bytes) == -1)
		{
			perror("[get_msg] write");
			break;
		}
	}
	return NULL;
}

void *send_msg(void *arg)
{
	int server_fd = *(int *)arg;
	char buffer[4096];

	ssize_t bytes;
	printf("Write some data:\n");
	while ((bytes = read(0, buffer, sizeof(buffer))) > 0)
	{
		if (write(server_fd, buffer, bytes) == -1)
		{
			perror("[send_msg] write");
			break;
		}
	}
	return NULL;
}

int main()
{
	int server_fd = connect_server();
	pthread_t p1;
	pthread_t p2;
	int rc;

	rc = pthread_create(&p1, NULL, get_msg, &server_fd);
	if (rc != 0)
	{
		fprintf(stderr, "pthread_create: %s\n", strerror(rc));
		close(server_fd);
		exit(1);
	}
	rc = pthread_create(&p2, NULL, send_msg, &server_fd);
	if (rc != 0)
	{
		fprintf(stderr, "pthread_create: %s\n", strerror(rc));
		close(server_fd);
		exit(1);
	}

	rc = pthread_join(p1, NULL);
	if (rc != 0)
	{
		fprintf(stderr, "pthread_join: %s\n", strerror(rc));
		close(server_fd);
		exit(1);
	}
	rc = pthread_join(p2, NULL);
	if (rc != 0)
	{
		fprintf(stderr, "pthread_join: %s\n", strerror(rc));
		close(server_fd);
		exit(1);
	}
}