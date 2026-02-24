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

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 17:07:24 by pauladretta       #+#    #+#             */
/*   Updated: 2026/02/16 00:40:35 by pdrettas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <cstring>
#include <cerrno>
#include <time.h>

enum PipeCloseCall
{
    CLOSE_ALL,
    CLOSE_SRV_TO_CGI, // closes both ends of pipe
    CLOSE_CGI_TO_SRV, // closes both ends of pipe
    CLOSE_READ_SRV_TO_CGI, // for parent logic
    CLOSE_WRITE_CGI_TO_SRV // for parent logic
};

/*
- envp: getting this variable from http parser 
    -> later convert to char** when using execve()
- add pid, pipe to class (in a class bc of possibly multiple requests at the same time, and concurrent CGI executions)
- this class says that this objects represents ONE CGI run
*/
class CgiHandler
{
    private:
        char** _envp; // from server
        std::string _requestBody; // from server: input/request (given to srv_to_cgi pipe[1]) (POST request, GET request (empty body)
        std::string _interpreterPath; // file that will be executed in cgi
        char **_argv; // filled w script name for execve (bc requires argv array)
        int _exitCode;
        pid_t _pid; // value child parent
        int _srv_to_cgi[2]; // server writes input [0], and cgi uses output [1]
        int _cgi_to_srv[2]; // cgi writes (output from script) in input [0], server uses output [1]
        std::string _cgiOutput;

    public:
        CgiHandler(const std::string& requestBody, char **envp, const std::string& filePath, char **argv);
        ~CgiHandler();
        bool validateExecveArgs(char **argv, char **envp);
        bool createPipes();
        void closePipeFds(PipeCloseCall action);
        bool redirectIO();
        bool writeRequestBodyToPipe();
        bool readCgiOutputFromPipe();
        bool execute();
        void waitAndGetExitCode();
        std::string buildHttpResponseFromCgi();
    };

#endif