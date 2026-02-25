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
#include <cerrno>
#include <time.h>
#include <cstring>

# include "CgiExceptions.hpp"

# define CGI_BUFFER_SIZE 1000
# define CGI_MAX_TIME 5

class Client;
class Epoll;

enum PipeCloseCall
{
    CLOSE_ALL,
    CLOSE_SRV_TO_CGI, // closes both ends of pipe
    CLOSE_CGI_TO_SRV, // closes both ends of pipe
    CLOSE_READ_SRV_TO_CGI, // for parent logic
    CLOSE_WRITE_CGI_TO_SRV // for parent logic
};

class CgiHandler
{
	private:
		int _srv_to_cgi[2]; // server writes input [0], and cgi uses output [1]
        int _cgi_to_srv[2]; // cgi writes (output from script) in input [0], server uses output [1]
		// int _infd[2];
		// int _outfd[2];
		std::string _requestBody; // from server: input/request (given to srv_to_cgi pipe[1]) (POST request, GET request (empty body)
        std::string _interpreterPath; // file that will be executed in cgi
		int _exitCode;
		std::string _cgiOutput;
		pid_t _pid; // value child parent
		// pid_t _execution_child;
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

		bool validateExecveArgs(char **argv, char **envp);
        bool createPipes();
        void closePipeFds(PipeCloseCall action);
        bool redirectIO();
        bool writeRequestBodyToPipe();
        bool readCgiOutputFromPipe();
        bool execute();
        void waitAndGetExitCode();
		void setInterpreterPath(const std::string& i_path);
};

#endif