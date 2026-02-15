#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> // для sockaddr_in, inet_pton
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>

int connect_socket()
{
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd == -1)
	{
		perror("socket");
		exit(1);
	}

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8080); // порт сервера

	if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
	{
		perror("inet_pton");
		close(sock_fd);
		exit(1);
	}

	if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		perror("connect");
		close(sock_fd);
		exit(1);
	}
	return sock_fd;
}

void *write_to_server(void *arg)
{
	int sock_fd = *(int *)arg;
	char buffer[4096];

	ssize_t bytes;
	printf("Enter some data:\n");
	while ((bytes = read(STDIN_FILENO, &buffer, sizeof(buffer))) > 0)
	{
		if (write(sock_fd, buffer, bytes) == -1)
		{
			perror("[write_to_server] write");
			break;
		}
	}
	return NULL;
}

void *read_from_server(void *arg)
{
	int sock_fd = *(int *)arg;
	char buffer[4096];

	ssize_t bytes;
	while ((bytes = read(sock_fd, buffer, sizeof(buffer))) > 0)
	{
		if (write(STDOUT_FILENO, buffer, bytes) == -1)
		{
			perror("[read_from_server] write");
			break;
		}
	}
	return NULL;
}

int main()
{
	int sock_fd = connect_socket();
	pthread_t p1;
	pthread_t p2;
	int rc;

	rc = pthread_create(&p1, NULL, write_to_server, &sock_fd);
	if (rc != 0)
	{
		fprintf(stderr, "pthread_create: %s\n", strerror(rc));
		close(sock_fd);
		exit(1);
	}
	rc = pthread_create(&p2, NULL, read_from_server, &sock_fd);
	if (rc != 0)
	{
		fprintf(stderr, "pthread_create: %s\n", strerror(rc));
		close(sock_fd);
		exit(1);
	}

	rc = pthread_join(p1, NULL);
	if (rc != 0)
	{
		fprintf(stderr, "pthread_join: %s\n", strerror(rc));
		close(sock_fd);
		exit(1);
	}
	rc = pthread_join(p2, NULL);
	if (rc != 0)
	{
		fprintf(stderr, "pthread_join: %s\n", strerror(rc));
		close(sock_fd);
		exit(1);
	}
	close(sock_fd);
}