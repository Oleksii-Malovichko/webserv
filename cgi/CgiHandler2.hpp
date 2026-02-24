/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler2.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tfarkas <tfarkas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 17:07:24 by pauladretta       #+#    #+#             */
/*   Updated: 2026/02/24 20:36:04 by tfarkas          ###   ########.fr       */
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