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

class Client;
class Epoll;

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
		size_t _sent_bytes;
		std::string _cgi_response;
		bool _stdin_closed;
		bool _stdout_closed;
		bool _child_exited;


	public:
		CgiHandler(void);

		~CgiHandler(void);

		void addArgsElement(
			std::string& value);
		void addEnvpElement(
			const std::string& key, const std::string& value);
		
		void runExecve(void);
		void readFromCgi(Epoll& epoll_obj);
		void writeToCgi(Epoll& epoll_obj,
			const std::string& request_body);
		void setEnvp(Client& client_obj);
		void setArgsAndCgiPath(char* in_cgi_path);
		void freeEnvp(void);
		void freeArgs(void);
		void printArgs(std::ostream& out) const;
		void printEnvp(std::ostream& out) const;
		void closePipeFd(int opt);
		void setNonBlockPipe(void);

		const char* getCgiPath(void) const;
		int getCgiInWriteFD(void) const;
		int getCgiOutReadFD(void) const;
		void finishChildProcess(void);
		bool IsCgiFinished(void);
		std::string buildCgiResponse(void);
		void terminateChild(void);
		
};

#endif