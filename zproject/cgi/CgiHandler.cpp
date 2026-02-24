/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 17:04:59 by pauladretta       #+#    #+#             */
/*   Updated: 2026/02/24 21:24:30 by pdrettas         ###   ########.fr       */
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
CgiHandler::CgiHandler(const std::string& requestBody, char **envp, const std::string& interpreterPath, char **argv)
    : _requestBody(requestBody), _envp(envp), _interpreterPath(interpreterPath), _argv(argv), _cgiOutput("")
{}

// destructor // TODO: cleanup
CgiHandler::~CgiHandler()
{}

// TODO: addEnvpElement ft
// void CgiHandler::setEnvp(Client& client_obj)
// {
//     this->addEnvpElement("REQUEST_METHOD", client_obj.getRequest().method);
//     this->addEnvpElement("SCRIPT_NAME", client_obj.getRequest().path);
//     this->addEnvpElement("PATH_INFO", "");
//     this->addEnvpElement("QUERY_STRING", client_obj.getRequest().query_string);
//     this->addEnvpElement("CONTENT_LENGTH", std::to_string(client_obj.getRequest().contentLength));
//     this->addEnvpElement("CONTENT_TYPE", "");
//     this->addEnvpElement("SERVER_PROTOCOL", client_obj.getRequest().version);

//     this->addEnvpElement("GATEWAY_INTERFACE", "CGI/1.1");
//     this->addEnvpElement("SERVER_SOFTWARE", "webserv/1.0");
//     this->addEnvpElement("HTTP_HOST", "localhost");
//     this->addEnvpElement("HTTP_USER_AGENT", "curl/7.88");
// }

// check if input from server is valid (ex. file accessible) before moving on to executing (argv, envp)
bool CgiHandler::validateExecveArgs(char **argv, char **envp)
{
    if (!envp || !argv || !argv[0])
    {
        std::cerr << "CGI Error: one of the execve arguments is NULL." << std::endl;
        return false;
    }
    
    if (access(argv[0], X_OK) != 0) // for executables
    {
        std::cerr << "CGI Error: cannot execute interpreter." << argv[0] << std::endl;
        return false;
    }

    if (access(argv[1], F_OK) != 0 || access(argv[1], R_OK) != 0) // for scripts
    {
        std::cerr << "CGI Error: script not found: " << argv[1] << std::endl;
        return false;
    }

    return true;
}

/*
Pipe 1: for server to write the request body to CGI
Pipe 2: for server to read output from CGI
*/
bool CgiHandler::createPipes()
{
    if ((pipe(_srv_to_cgi) == -1) || (pipe(_cgi_to_srv) == -1))
    {
        std::cerr << "CGI Error: pipe could not be created." << std::endl;
        return false;
    }
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
        std::cerr << "CGI Error: dup2 failed." << std::endl;
        this->closePipeFds(CLOSE_ALL);
        return false;
    }
    this->closePipeFds(CLOSE_SRV_TO_CGI);
    
    // redirect output: CGI writes to second pipe
    if (dup2(this->_cgi_to_srv[1], STDOUT_FILENO) == -1)
    {
        std::cerr << "CGI Error: dup2() failed." << std::endl;
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
        {
            std::cerr << "CGI Error: write() failed." << std::endl;
            return false;
        }
    }
    close(this->_srv_to_cgi[1]); // signals end of input to CGI to avoid waiting/blocking, TODO: replace w closepipe ft later
    return true;
}

// ft: read the CGI output (from the child process that executed the file script) from the second pipe (cgi to srv[0]) (CGI -> server)
// added: pipe needs to be non-blocking bc an infinite loop will keep read() forever and pipe wont be closed
bool CgiHandler::readCgiOutputFromPipe()
{
    int bytesRead = 1;
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

// ft: waits for child to finish or kill child if stuck too long (ex. infinite loop) and captures the exit code 
// added: add non-blocking (ex. infinite loop script) by doing a timeout
void CgiHandler::waitAndGetExitCode()
{
    int status = 0;
    time_t start = time(NULL); // ex. start 4:10:00 pm 
    
    while (1) // loop needed to continously recheck the ongoing time that has passed until x seconds reached to kill
    {
        pid_t result = waitpid(this->_pid, &status, WNOHANG); // checks if child exited (WNOHANG: checks without blocking)
        if (result > 0) // if child exited
        {
            if (WIFEXITED(status)) // exited normally
                this->_exitCode = WEXITSTATUS(status);
            else if (WIFSIGNALED(status)) // killed by signal 
                this->_exitCode = 1;
            break;
        }
        if (difftime(time(NULL), start) >= 5) // compares btw beginning time and now how much has passed
        {
            kill(this->_pid, SIGKILL); // kill process
            waitpid(this->_pid, &status, 0); // cleanup, no zombies
            this->_exitCode = 124;
            std::cerr << "CGI Error: timeout reached." << std::endl;
            break;
        }
        usleep(100000); // sleep 100ms to avoid busy waiting / burning CPU
    }
    std::cout << "Exit Code: " << this->_exitCode << std::endl; // TODO: delete after testing
}

/*
MAIN execution function (will be called in handleCGI ft)
NEXT STEPS TODO:
1. translate CGI output -> HTTP response
2. add more testing scripts (python, golang, c/c++ if wanted)
3. **DONE waitpid and check exit code/status as well (idk: change from 0 to WOHANG)
4. limit cgi execution time: add timeout protection (w alarm(30) ex. kill after 30 seconds)
5. error handling: add more descriptive error messages (for debugging) (ex. "fork failed")
6. handle large outputs ?

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
        execve(this->_interpreterPath.c_str(), this->_argv, this->_envp);
        std::cerr << "CGI Error: execve() failed." << std::endl;
        exit(1);
    }
    else // parent process ("server")
    {
        this->closePipeFds(CLOSE_READ_SRV_TO_CGI); // parent does not read CGI stdin
        this->closePipeFds(CLOSE_WRITE_CGI_TO_SRV); // parent does not write CGI stdout
        
        if (!this->writeRequestBodyToPipe())
            return false;
            
        this->waitAndGetExitCode();
            
        if (!this->readCgiOutputFromPipe())
            return false;

    }
    return true;
}       

// TODO: ft: translate cgi output as http response
/* The CGI script only produces headers and body content.
It doesn't return full HTTP.
->Bc CGI is an interface btw the web server & external programs.
->The web server is responsible for the HTTP protocol.

structure of an HTTP response:
1. valid HTTP status line (status-code could be 200, 404,...)
2. headers
3. a blank line
4. the body

HTTP/1.1 <status-code> <reason-phrase>\r\n
Header: value\r\n (content-type)
Header: value\r\n (content-length (could take bytesread))
\r\n
<body>

variables from cgi class needed:
- cgioutput, exitcode
(if 1 -> status code 500 Internal Server Error. if 0 -> status code 200 OK)

DO NOT PUT IN HANDLE CLIENT FT ANYMORE. ALL CGI WILL BE PUT INTO main of webserv.
notes: handle status codes maybe outside cgi (bc sometimes cgi will not run depending on method)
*/
std::string CgiHandler::buildHttpResponseFromCgi()
{
    std::string httpResponse;





    std::cout << httpResponse << std::endl;

    return httpResponse;
}
