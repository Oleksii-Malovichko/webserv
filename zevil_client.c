gg#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h> // для sockaddr_in, inet_pton
#include <sys/socket.h>

volatile sig_atomic_t run = 1;

// pthread_mutex_t run_mutex = PTHREAD_MUTEX_INITIALIZER; 

#define MAX_THREAD 1000

// void call_cmd(char **cmd)
// {
// 	pid_t pid = fork();
// 	if (pid == 0)
// 	{
// 		execvp(cmd[0], cmd);
// 		perror("execvp");
// 		exit(1);
// 	}
// 	else if (pid > 0)
// 	{
// 		int status;
// 		waitpid(pid, &status, 0);
// 		if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
// 		{
// 			fprintf(stderr, "Process finished with problem");
// 			exit(1);
// 		}
// 		else if (WIFSIGNALED(status))
// 		{
// 			fprintf(stderr, "Process killed by signal%d\n", WTERMSIG(status));
// 			exit(1);
// 		}
// 		printf("Process finished nice\n");
// 	}
// 	else
// 	{
// 		perror("fork");
// 		exit(1);
// 	}
// }

void handle_sig(int sig)
{
	(void)sig;
	run = 0;
}

void *thread_handler(void *arg)
{
	char **cmd = (char **)arg;
	(void)cmd;
	while (run)
	{
		int fd = socket(AF_INET, SOCK_STREAM, 0);
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(8080);
		if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0)
		{
			perror("inet_pton");
			close(fd);
			exit(1);
		}
		if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
		{
			if (errno == EADDRNOTAVAIL)
			{
				close(fd);
				continue;
			}
			perror("connect");
			close(fd);
			exit(1);
		}
		const char *body = "name=John+Doe&age=30&city=NY";

		char buffer[1024];

		int len = snprintf(buffer, sizeof(buffer),
			"POST /submit-form HTTP/1.1\r\n"
			"Host: localhost:8080\r\n"
			"User-Agent: stress-client\r\n"
			"Accept: */*\r\n"
			"Content-Type: application/x-www-form-urlencoded\r\n"
			"Content-Length: %zu\r\n"
			"\r\n"
			"%s",
			strlen(body),
			body
		);
		send(fd, buffer, len, 0);
		close(fd);
	}
	return NULL;
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "USAGE: %s curl http...\n", argv[0]);
		return 1;
	}
	signal(SIGINT, handle_sig);
	pthread_t p_arr[MAX_THREAD];
	int i = 0;
	int rc;
	while (i < MAX_THREAD)
	{
		rc = pthread_create(&p_arr[i], NULL, thread_handler, &argv[1]);
		if (rc != 0)
		{
			fprintf(stderr, "pthread_create: %s\n", strerror(rc));
			exit(1);
		}
		i++;
	}
	i = 0;
	while (i < MAX_THREAD)
	{
		rc = pthread_join(p_arr[i], NULL);
		if (rc != 0)
		{
			fprintf(stderr, "pthread_join: %s\n", strerror(rc));
			exit(1);
		}
		i++;
	}
}