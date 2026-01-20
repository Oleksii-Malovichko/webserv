#include "errorHandler.hpp"
#include <fcntl.h>

// c++ -Wall -Werror -Wextra -std=c++17  errorHandler.cpp test_errorHandler.cpp -o test_errorHandler

int main(void)
{
	int fd = open("try.txt", O_RDONLY);
	//int fd = open("non_existent_file.txt", O_RDONLY);
	if (fd == -1)
	{
		errorHandler error_msg("Failed to open file");
		int error_code = error_msg.getErrorCode();
		error_msg.printStderr();
		return (error_code);
	}

	errorHandler test1(100);
	test1.printFd(STDERR_FILENO);
	errorHandler test2(400);
	test2.printFd(STDERR_FILENO);

	close(fd);
	return (0);
}