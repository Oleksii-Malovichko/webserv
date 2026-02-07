/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 17:04:59 by pauladretta       #+#    #+#             */
/*   Updated: 2026/02/07 22:09:05 by pdrettas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"

// function is from main code base empty (entry point of CGI) already
// (fill at end when cgi part done -> this ft is the start point for CGI)
// calls my CGI executor, result of that will be sent back to client
// void Server::handleCGI(Client &client)
// {
//     // CGIRequest req = buildFromClient(client);
//     // CGIResult res = cgi.execute(req);
//     // sendHttpResponse(client, res);
// }

// constructor
CgiHandler::CgiHandler()
{
    // initialize variables here
}

// destructor
CgiHandler::~CgiHandler()
{}

// execution function (will be called in handleCGI ft)
/*
- redirect stdin only for POST & PUT requests, not necessary for GET requests
- 2 pipes


NEXT STEPS:
1. finish redirection in child, and waitpid
2. create example of envp (also see function below below)
3. test w testscript.py when calling execute ft in main
4. write ft for closing fds
*/
bool CgiHandler::execute()
{
    // this is a rough draft
    // create 2 pipes w file descriptors
    if ((pipe(_in_cgi) == -1) || (pipe(_out_cgi) == -1))
        return 0; // bool false
    
    // fork
    this->_pid = fork();
    if (this->_pid == -1)
    {
        close (this->_in_cgi[0]);
        close (this->_in_cgi[1]);
        close (this->_out_cgi[0]);
        close (this->_out_cgi[1]);
        return 0; // bool false
    }
    
    // child
    if (this->_pid == 0)
    {
        // redirect input (recheck meaning of this)
        if (dup2(this->_in_cgi[0], STDIN_FILENO) == -1)
        {
            close (this->_in_cgi[0]);
            close (this->_in_cgi[1]);
            close (this->_out_cgi[0]);
            close (this->_out_cgi[1]);
            exit(1);
        }
        
        // redirect output (recheck meaning of this)
        if (dup2(this->_out_cgi[1], STDOUT_FILENO) == -1)
        {
            close (this->_in_cgi[0]);
            close (this->_in_cgi[1]);
            close (this->_out_cgi[0]);
            close (this->_out_cgi[1]);
            exit(1);
        }
        
        // execve CGI (TODO: envp -> create example of envp to use here)
        exit(1);
    }
    
    // parent
        // close fds
        // waitpid
        // ...
        // write request body (post only)
        // read cgi output)

    return 1; // bool true
}       



// T ft
// void CgiHandler::setEnvp(void)
// {
// 	this->addEnvpElement("REQUEST_METHOD", "GET");
// 	this->addEnvpElement("SCRIPT_NAME", "/cgi-bin/test.py");
// 	this->addEnvpElement("PATH_INFO", "");
// 	this->addEnvpElement("QUERY_STRING", "name=Tom&age=25");
// 	this->addEnvpElement("CONTENT_LENGTH", "");
// 	this->addEnvpElement("CONTENT_TYPE", "");
// 	this->addEnvpElement("SERVER_PROTOCOL", "HTTP/1.1");

// 	this->addEnvpElement("GATEWAY_INTERFACE", "CGI/1.1");
// 	this->addEnvpElement("SERVER_SOFTWARE", "webserv/1.0");
// 	this->addEnvpElement("HTTP_HOST", "localhost");
// 	this->addEnvpElement("HTTP_USER_AGENT", "curl/7.88");
// }


