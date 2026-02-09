/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 17:04:59 by pauladretta       #+#    #+#             */
/*   Updated: 2026/02/09 12:11:57 by pdrettas         ###   ########.fr       */
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
CgiHandler::CgiHandler(const std::string& requestBody, char **envp, const std::string& filePath, char **argv)
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
    if (action == CLOSE_READ_SRV_TO_CGI)
    {
        close(_srv_to_cgi[0]);
    }
    if (action == CLOSE_WRITE_CGI_TO_SRV)
    {
        close(_cgi_to_srv[1]);
    }
}

// execution function (will be called in handleCGI ft)
/*
Pipe 1: for server to write the request body to CGI
Pipe 2: for server to read output from CGI

NEXT STEPS TODO:
- **DONE 1: write ft for closing fds
- **DONE 2: finish redirection in child, and waitpid
- **DONE 2: create testing main (envp, body)
- **DONE 2: add variables to personalized constructor coming from server, needed for input/execution

- **DONE 3: add execve part
- **DONE 3: add write part
- **DONE 3: add read part

- test w testscript.py when calling execute ft in main

- translate CGI output -> HTTP response

- refactor execute ft by putting parts into helper fts
- add error stuff
- (?) check waitpid status / exit code
- (?) reorder closing fds for child based on parent-only & child-only logic

- recheck file descriptors if closing is timed properly 
- recheck error handling (messages, etc) (possibly instead of exit)
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
        
        // execve CGI
        execve(this->_filePath.c_str(), this->_argv, this->_envp);
        std::cout << "cgi execution error" << std::endl; // TODO: delete after testing
        exit(1);
    }
    // parent ("server")
    else
    {
        // close fds
        this->closePipes(CLOSE_READ_SRV_TO_CGI); // parent does not read CGI stdin
        this->closePipes(CLOSE_WRITE_CGI_TO_SRV); // parent does not write CGI stdout
        
        // write POST(full) & GET(empty) request body to the first pipe shared w CGI (server -> CGI)
            // TODO: maybe add this above the child (for better reading order)
            // for GET request body is empty, so only write() if body not empty
        if (!this->_requestBody.empty())
        {
            if (write(this->_srv_to_cgi[1], this->_requestBody.c_str(), this->_requestBody.size()) == -1) // fd to pipe, pointer to data/bytes, number of bytes to write
                return false;
        }
        close(this->_srv_to_cgi[1]); // signals end of input to CGI to avoid waiting/blocking, TODO: replace w closepipe ft later
        
        
        // read the CGI output (from the child process that executed the file script) from the second pipe (cgi to srv[0]) (CGI -> server)
        int bytesRead;
        char buffer[4096];
        std::string cgiOutput;
        while (bytesRead > 0)
        {
            bytesRead = read(this->_cgi_to_srv[0], buffer, sizeof(buffer));
            cgiOutput.append(buffer, bytesRead);
        }
        if (bytesRead == -1) // error check for read ft
            return false;
        
        close (this->_cgi_to_srv[0]); // close bc done using, TODO: replace w closepipe ft later

        std::cout << "CGI OUTPUT: " << std::endl << cgiOutput << std::endl;
        
        
        // waitpid
        waitpid(this->_pid, NULL, 0); // no zombies // TODO: check exit status?

        
        // translate cgi output as http response
        
            

    }

    return true;
}       
