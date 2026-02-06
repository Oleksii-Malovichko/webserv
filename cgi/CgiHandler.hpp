#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include <chrono>
# include <unistd.h>
# include <iostream>
# include <string>

# include "CgiExceptions.hpp"

# define CGI_BUFFER_SIZE 1000

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
		std::chrono::steady_clock::time_point 
			_execution_start_time;
		

	public:
		CgiHandler(void);

		~CgiHandler(void);

		int addArgsElement(
			std::string& value);
		int addEnvpElement(
			const std::string& key, const std::string& value);
		
		int runExecve(void) const;
		void setEnvp(void); // later the request class will be the argument
		void printArgs(std::ostream& out) const;
		void printEnvp(std::ostream& out) const;
		const char* getCgiPath(void) const;
		
};

#endif