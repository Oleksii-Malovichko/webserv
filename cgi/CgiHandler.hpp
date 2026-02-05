#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include <chrono>
# include <unistd.h>

class CgiHandler
{
	private:
		int _infd[2];
		int _outfd[2];
		pid_t _execution;
		char **_envp;
		char **args;
		std::chrono::steady_clock::time_point 
			_execution_start_time;
		

	public:
		CgiHandler(void);

		~CgiHandler(void);
};

#endif