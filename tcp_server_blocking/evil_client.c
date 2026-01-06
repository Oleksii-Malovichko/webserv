#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> // для sockaddr_in, inet_pton
#include <sys/socket.h>

int main()
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

	char *buffer = malloc(64000);
	if (!buffer)
	{
		perror("malloc");
		exit(1);
	}
	memset(buffer, 'A', 64000);
	int i = 0;
	while (1)
	{
		if (write(sock_fd, buffer, 64000) == -1)
		{
			perror("write");
			break;
		}
		printf("Chunks sent: %d\n", i);
		i++;
	}
	free(buffer);
	close(sock_fd);
	return 0;
}