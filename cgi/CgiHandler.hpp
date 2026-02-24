#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include <chrono>
# include <unistd.h>
# include <signal.h>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <iostream>
# include <string>

# include "CgiExceptions.hpp"

# define CGI_BUFFER_SIZE 1000
# define CGI_MAX_TIME 5

class CgiHandler
{
	private:
		int _infd[2];
		int _outfd[2];
		pid_t _execution_child;
		char* cgi_path;
		int _args_num;
		int _envp_num;
		char** _args;
		char** _envp;

	public:
		CgiHandler(void);

		~CgiHandler(void);

		void addArgsElement(
			std::string& value);
		void addEnvpElement(
			const std::string& key, const std::string& value);
		
		static void cgi_timout_handler(int sig) noexcept;
		
		std::string runExecve(void);
		void setEnvp(void); // later the request class will be the argument
		void setArgsAndCgiPath(char* in_cgi_path);
		void freeEnvp(void);
		void freeArgs(void);
		void printArgs(std::ostream& out) const;
		void printEnvp(std::ostream& out) const;
		void closePipeFd(int opt);
		void setNonBlockPipe(void);
		const char* getCgiPath(void) const;
		
};

