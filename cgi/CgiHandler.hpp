/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 17:07:24 by pauladretta       #+#    #+#             */
/*   Updated: 2026/02/09 09:47:49 by pdrettas         ###   ########.fr       */
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

enum PipeCloseCall
{
    CLOSE_ALL,
    CLOSE_SRV_TO_CGI, // closes both ends of pipe
    CLOSE_CGI_TO_SRV, // closes both ends of pipe
    CLOSE_SRV_TO_CGI_RD, // for parent logic
    CLOSE_CGI_TO_SRV_WR // for parent logic
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
        std::string _filePath; // file that will be executed in cgi
        char **_argv; // filled w script name for execve (bc requires argv array)
        // time
        // exit code (use later)
        // ...
        pid_t _pid; // value child parent
        int _srv_to_cgi[2]; // server writes input [0], and cgi uses output [1]
        int _cgi_to_srv[2]; // cgi writes (output from script) in input [0], server uses output [1]

    public:
        CgiHandler(std::string& requestBody, char ** envp, std::string filePath, char **argv);
        ~CgiHandler();
        bool execute();
        void closePipes(PipeCloseCall action);
};

#endif