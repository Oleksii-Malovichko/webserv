#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> // для sockaddr_in, inet_pton
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>

#define MAX_BYTES 4096

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
	return server_fd;
}

void *read_data(void *arg)
{
	char buffer[MAX_BYTES];
	int server_fd = *(int *)arg;

	ssize_t bytes;
	while ((bytes = read(server_fd, buffer, sizeof(buffer))) > 0)
	{
		if (write(STDOUT_FILENO, buffer, bytes) == -1)
		{
			perror("[read_data] write");
			break;
		}
	}
	return NULL;
}

void *write_data(void *arg)
{
	char buffer[MAX_BYTES];
	int server_fd = *(int *)arg;

	ssize_t bytes;
	printf("Enter some data:\n");
	while ((bytes = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0)
	{
		if (write(server_fd, buffer, bytes) == -1)
		{
			perror("[write_data] write");
			break;
		}
	}
	return NULL;
}

int main()
{
	pthread_t p1;
	pthread_t p2;
	int rc;
	int server_fd = connect_server();

	rc = pthread_create(&p1, NULL, write_data, &server_fd);
	if (rc != 0)
	{
		fprintf(stderr, "pthread_create: %s\n", strerror(rc));
		close(server_fd);
		exit(1);
	}
	rc = pthread_create(&p2, NULL, read_data, &server_fd);
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
	return 0;
}
