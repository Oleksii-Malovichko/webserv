/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 17:04:59 by pauladretta       #+#    #+#             */
/*   Updated: 2026/02/09 09:48:12 by pdrettas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"

// function is from main code base empty (entry point of CGI) already
// (fill at end when cgi part done -> this ft is the start point for CGI)
// calls my CGI executor, result of that will be sent back to client
// void Server::handleCGI(Client &client)
// {
        // execute ft will be called here
// }

// personalized constructor
CgiHandler::CgiHandler(std::string& requestBody, char **envp, std::string filePath, char **argv)
    : _requestBody(requestBody), _envp(envp), _filePath(filePath), _argv(argv)
{}

// destructor
// TODO: cleanup
CgiHandler::~CgiHandler()
{}

void CgiHandler::closePipes(PipeCloseCall action)
{
    if (action == CLOSE_ALL || action == CLOSE_SRV_TO_CGI)
    {
        close(this->_srv_to_cgi[0]);
        close(this->_srv_to_cgi[1]);
    }
    if (action == CLOSE_ALL || action == CLOSE_CGI_TO_SRV)
    {
        close(this->_cgi_to_srv[0]);
        close(this->_cgi_to_srv[1]);
    }
    if (action == CLOSE_SRV_TO_CGI_RD)
    {
        close(_srv_to_cgi[0]);
    }
    if (action == CLOSE_CGI_TO_SRV_WR)
    {
        close(_cgi_to_srv[1]);
    }
}

// execution function (will be called in handleCGI ft)
/*
- redirect stdin only for POST requests, not necessary for GET requests

NEXT STEPS TODO:
- **DONE 2: finish redirection in child, and waitpid
- create testing main (envp, body)
- **DONE 2: add variables to personalized constructor coming from server, needed for input/execution
- test w testscript.py when calling execute ft in main
- **DONE 1: write ft for closing fds

- Translate CGI output -> HTTP response

- refactor execute ft by putting parts into helper fts
- add error stuff
- (?) check waitpid status / exit code
- (?) reorder closing fds for child based on parent-only & child-only logic

- recheck file descriptors if closing is timed properly 
*/
bool CgiHandler::execute()
{
    // this is a rough draft
    // create 2 pipes w file descriptors
    if ((pipe(_srv_to_cgi) == -1) || (pipe(_cgi_to_srv) == -1))
        return false;
    
    // fork
    this->_pid = fork();
    if (this->_pid == -1)
    {
        this->closePipes(CLOSE_ALL);
        return false; 
    }
    
    // child
    if (this->_pid == 0)
    {
        // redirect input: CGI reads from pipe
        if (dup2(this->_srv_to_cgi[0], STDIN_FILENO) == -1)
        {
            this->closePipes(CLOSE_ALL);
            exit(1);
        }
        this->closePipes(CLOSE_SRV_TO_CGI);
        
        // redirect output: CGI writes to pipe
        if (dup2(this->_cgi_to_srv[1], STDOUT_FILENO) == -1)
        {
            this->closePipes(CLOSE_ALL);
            exit(1);
        }
        this->closePipes(CLOSE_CGI_TO_SRV);
        
        // execve CGI (TODO: envp -> create example of envp to use here)
        exit(1);
    }
    // parent
    else
    {
            // close fds
            this->closePipes(CLOSE_SRV_TO_CGI_RD); // parent does not read CGI stdin
            this->closePipes(CLOSE_CGI_TO_SRV_WR); // parent does not write CGI stdout
            
            // write() POST request body (form data, json, etc) (srv to cgi[1]): writing the body to the pipe
                // TODO: maybe add this above the child (for better reading order)
                // close write end once done so CGI knows no more data coming
                // for GET request body is empty
            


            
            
            // read() CGI stdout  (cgi to srv[0])


            

            
            
            // waitpid
            waitpid(this->_pid, NULL, 0); // no zombies // TODO: check exit status?

            // translate cgi output as http response
            
            

            
    }

    return true;
}       
