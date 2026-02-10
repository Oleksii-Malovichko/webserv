/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 17:04:59 by pauladretta       #+#    #+#             */
/*   Updated: 2026/02/10 20:51:18 by pdrettas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"

// function is from main code base empty (entry point of CGI) already
// (fill at end when cgi part done -> this ft is the start point for CGI in the entire webserv project)
// calls my CGI executor, result of that will be sent back to client
// TODO: add the contents of this function to alex's part in his ft handleCGI after merging 
// void Server::handleCGI(Client &client)
// {
        // TODO: change input of constructor w those of Client
        // CgiHandler cgi(postBody, envp, argv[1], argv);
        // if (!cgi.execute())
        // {
        //     std::cerr << "CGI execution failed!" << std::endl;
        //     return 1;
        // }
        // TODO: add ft that translates CGI output to HTTP response
// }

// personalized constructor
CgiHandler::CgiHandler(const std::string& requestBody, char **envp, const std::string& filePath, char **argv)
    : _requestBody(requestBody), _envp(envp), _filePath(filePath), _argv(argv), _cgiOutput("")
{}

// destructor // TODO: cleanup
CgiHandler::~CgiHandler()
{}

// check if input from server is valid (ex. file accessible) before moving on to executing (argv, envp)
bool CgiHandler::validateExecveArgs(char **argv, char **envp)
{
    if (!envp || !argv || !argv[0])
        return false;
    
    if (access(argv[0], X_OK) != 0) // for executables
        return false;

    if (access(argv[1], F_OK) != 0) // for scripts
        return false;

    return true;
}

/*
Pipe 1: for server to write the request body to CGI
Pipe 2: for server to read output from CGI
*/
bool CgiHandler::createPipes()
{
    if ((pipe(_srv_to_cgi) == -1) || (pipe(_cgi_to_srv) == -1))
        return false;
    return true;
}

// ft: close file descriptors of pipes accordingly
void CgiHandler::closePipeFds(PipeCloseCall action)
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

// ft: redirection
bool CgiHandler::redirectIO()
{
    // redirect input: CGI reads from first pipe
    if (dup2(this->_srv_to_cgi[0], STDIN_FILENO) == -1)
    {
        this->closePipeFds(CLOSE_ALL);
        return false;
    }
    this->closePipeFds(CLOSE_SRV_TO_CGI);
    
    // redirect output: CGI writes to second pipe
    if (dup2(this->_cgi_to_srv[1], STDOUT_FILENO) == -1)
    {
        this->closePipeFds(CLOSE_ALL);
        return false;
    }
    this->closePipeFds(CLOSE_CGI_TO_SRV);
    return true;
}

// ft: write POST(full) & GET(empty) request body to the first pipe shared w CGI (server -> CGI)
bool CgiHandler::writeRequestBodyToPipe()
{
    if (!this->_requestBody.empty())
    {
        if (write(this->_srv_to_cgi[1], this->_requestBody.c_str(), this->_requestBody.size()) == -1) // fd to pipe, pointer to data/bytes, number of bytes to write
            return false;
    }
    close(this->_srv_to_cgi[1]); // signals end of input to CGI to avoid waiting/blocking, TODO: replace w closepipe ft later
    return true;
}

// ft: read the CGI output (from the child process that executed the file script) from the second pipe (cgi to srv[0]) (CGI -> server)
bool CgiHandler::readCgiOutputFromPipe()
{
    int bytesRead;
    char buffer[4096];
    
    while (bytesRead > 0)
    {
        bytesRead = read(this->_cgi_to_srv[0], buffer, sizeof(buffer));
        _cgiOutput.append(buffer, bytesRead);
    }
    if (bytesRead == -1) // error check for read ft
        return false;
    close (this->_cgi_to_srv[0]); // close bc done using, TODO: replace w closepipe ft later
    
    std::cout << "CGI OUTPUT: " << std::endl << this->_cgiOutput << std::endl; // TODO: delete after testing
    return true;
}

// ft: waits for child to finish and captures the exit code 
void CgiHandler::waitAndGetExitCode()
{
    int status;
    
    waitpid(this->_pid, &status, 0);
    if (WIFEXITED(status))
        this->_exitCode = WEXITSTATUS(status);
    std::cout << "Exit Code: " << this->_exitCode << std::endl; // TODO: delete after testing
}

/*
MAIN execution function (will be called in handleCGI ft)
NEXT STEPS TODO:
1. **DONE check if file / argv is accessible (w access ft) before execute ft
2. translate CGI output -> HTTP response
3. **DONE waitpid and check exit code/status as well (idk: change from 0 to WOHANG)
4. add more testing scripts (python, golang, c/c++ if wanted)

- rearrange/rename closing fds w function based on parent-only & child-only logic
- recheck error handling (messages, etc) (possibly instead of exit)
*/
bool CgiHandler::execute()
{
    if (!createPipes())
        return false;

    this->_pid = fork();
    if (this->_pid == -1)
    {
        this->closePipeFds(CLOSE_ALL);
        return false; 
    }
    
    if (this->_pid == 0) // child process
    {
        if (!this->redirectIO())
            exit(1);
        execve(this->_filePath.c_str(), this->_argv, this->_envp);
        exit(1);
    }
    else // parent process ("server")
    {
        this->closePipeFds(CLOSE_READ_SRV_TO_CGI); // parent does not read CGI stdin
        this->closePipeFds(CLOSE_WRITE_CGI_TO_SRV); // parent does not write CGI stdout
        
        if (!this->writeRequestBodyToPipe())
            return false;
            
        if (!this->readCgiOutputFromPipe())
            return false;

        this->waitAndGetExitCode();
    }

    return true;
}       

// TODO: ft: translate cgi output as http response
