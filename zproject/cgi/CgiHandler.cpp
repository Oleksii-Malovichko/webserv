#include "CgiHandler.hpp"
#include "../Client/Client.hpp"
#include "../RAII/epoll/Epoll.hpp"

CgiHandler::CgiHandler(void)
{
	this->_srv_to_cgi[0] = -1;
	this->_srv_to_cgi[1] = -1;
	this->_cgi_to_srv[0] = -1;
	this->_cgi_to_srv[1] = -1;
	this->_args_num = 0;
	this->_envp_num = 0;
	this->_envp = NULL;
	this->_args = NULL;
	this->_requestBody.clear(); // setting to empty ""
	this->_interpreterPath.clear();
	this->_exitCode = 0;
	this->_cgiOutput.clear();
	this->_sent_bytes = 0;
	this->_cgi_response = "";
	this->_stdin_closed = false;
	this->_stdout_closed = false;
	this->_child_exited = false;
	this->cgi_path = NULL;
	this->_pid = -1;

	if (!createPipes())
        throw pipeError("Failed to create output pipe");
	
}

CgiHandler::~CgiHandler(void)
{
	this->freeArgs();
	this->_args_num = 0;
	this->freeEnvp();
	this->_envp_num = 0;
	this->_sent_bytes = 0;
	std::cerr << CYAN << "Deleting CgiHandler this=" // new
				<< this << DEFAULT << std::endl;
}

void CgiHandler::freeEnvp(void)
{
	for (int i = 0; this->_envp && this->_envp[i]; i++)
	{
		free(this->_envp[i]);
	}
	free(this->_envp);
	this->_envp = NULL;
}

void CgiHandler::freeArgs(void)
{
	for (int i = 0; this->_args && this->_args[i]; i++)
	{
		free(this->_args[i]);
	}
	free(this->_args);
	this->_args = NULL;
}

void CgiHandler::addArgsElement(std::string& value)
{
	this->_args_num++;
	int i = 0;

	char **new_args = (char**)malloc((this->_args_num + 1) * sizeof(char*));

	while (this->_args_num > 1 && i < this->_args_num -1)
	{
		new_args[i] = strdup(this->_args[i]);
		i++;
	}
	
	new_args[i] = strdup(value.c_str());
	new_args[this->_args_num] = NULL;
	this->freeArgs();
	this->_args = new_args;
}

void CgiHandler::addEnvpElement(const std::string& key,
	const std::string& value)
{
	std::string add_str = key + "=" + value;
	this->_envp_num++;
	int i = 0;

	char **new_envp = (char **)malloc((this->_envp_num + 1) * sizeof(char*));

	while (this->_envp_num > 1 && i < this->_envp_num - 1)
	{
		new_envp[i] = strdup(this->_envp[i]);
		i++;
	}

	new_envp[i] = strdup(add_str.c_str());
	new_envp[this->_envp_num] = NULL;
	this->freeEnvp();
	this->_envp = new_envp;
}	


// void CgiHandler::runExecve(void)
// {
// 	this->_execution_child = fork();
// 	if (this->_execution_child == -1 )
// 		throw forkError();
	
// 	if (_execution_child == 0)
// 	{
// 		if (dup2(this->_infd[0], STDIN_FILENO) == -1)
// 			throw dup2Error("Failed to dup2 _infd[0]");

// 		if (dup2(this->_outfd[1], STDOUT_FILENO) == -1)
// 			throw dup2Error("Failed to dup2 _infd[0]");

// 		this->closePipeFd(0);
		
// 		execve("/usr/bin/python3", 
// 			this->_args, this->_envp);
// 		throw execveError(*this);
// 	}
// 	else
// 	{
// 		this->closePipeFd(1);
// 	}
// }


// void CgiHandler::readFromCgi(Epoll& epoll_obj)
// {
// 	char buffer[CGI_BUFFER_SIZE + 1];
// 	int readbyte = 0;
	
// 	while (true)
// 	{
// 		readbyte = read(this->_outfd[0], buffer, CGI_BUFFER_SIZE);
// 		if (readbyte > 0)
// 		{
// 			this->_cgi_response.append(buffer, readbyte);
// 		}
// 		else if (readbyte == 0)
// 		{
// 			epoll_obj.removeCgiFd(this->_outfd[0]);
// 			close(this->_outfd[0]);
// 			this->_stdout_closed = true;
// 			break ;
// 		}
// 		else
// 		{
// 			if (errno == EAGAIN || errno == EWOULDBLOCK)
// 			{
// 				break ;
// 			}
// 			else
// 			{
// 				throw readError(*this);
// 			}
// 		}
// 	}
// }

void CgiHandler::readFromCgi(Epoll& epoll_obj)
{
	char buffer[CGI_BUFFER_SIZE + 1];
	int readbyte = 0;
	
	while (true)
	{
		readbyte = read(this->_cgi_to_srv[0], buffer, CGI_BUFFER_SIZE);
		if (readbyte > 0)
		{
			this->_cgi_response.append(buffer, readbyte);
		}
		else if (readbyte == 0)
		{
			epoll_obj.removeCgiFd(this->_cgi_to_srv[0]);
			close(this->_cgi_to_srv[0]);
			this->_stdout_closed = true;
			break ;
		}
		else
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				break ;
			}
			else
			{
				throw readError(*this);
			}
		}
	}
}

// void CgiHandler::writeToCgi(Epoll& epoll_obj,
// 	const std::string& request_body)
// {
// 	ssize_t bytes = write(this->_infd[1], 
// 		request_body.c_str() + this->_sent_bytes,
// 		request_body.length() - this->_sent_bytes);

// 	if (bytes > 0)
// 	{
// 		this->_sent_bytes += bytes;

// 		if (this->_sent_bytes == request_body.length())
// 		{
// 			epoll_obj.removeCgiFd(this->_infd[1]);
// 			close(this->_infd[1]);
// 			this->_stdin_closed = true;
// 		}
// 	}
// 	else if (bytes == -1 && errno == EAGAIN)
// 	{
// 		return ;
// 	}
// 	else
// 	{
// 		throw writeError(*this);
// 	}
// }

void CgiHandler::writeToCgi(Epoll& epoll_obj,
	const std::string& request_body)
{
	if (request_body.size() == 0)
	{
		std::cerr << RED << "The request body empty"
					<< DEFAULT << std::endl;
		epoll_obj.removeCgiFd(this->_srv_to_cgi[1]);
		close(this->_srv_to_cgi[1]);
		this->_stdin_closed = true;
		return ;
	}
	
	ssize_t bytes = write(this->_srv_to_cgi[1], 
		request_body.c_str() + this->_sent_bytes,
		request_body.length() - this->_sent_bytes);

	if (bytes > 0)
	{
		this->_sent_bytes += bytes;

		if (this->_sent_bytes == request_body.length())
		{
			epoll_obj.removeCgiFd(this->_srv_to_cgi[1]);
			close(this->_srv_to_cgi[1]);
			this->_stdin_closed = true;
		}
	}
	else if (bytes == -1 && errno == EAGAIN)
	{
		return ;
	}
	else
	{
		throw writeError(*this);
	}
}

// void CgiHandler::finishChildProcess(void)
// {
// 	int status;
// 	pid_t res = waitpid(this->_execution_child, &status, WNOHANG);

// 	if (res == 0)
// 		return ;
// 	else if (res == -1)
// 	{
// 		throw execveError(*this);
// 	}
// 	else
// 	{
// 		if (WIFEXITED(status))
// 		{
// 			if (WEXITSTATUS(status) != 0 || WIFSIGNALED(status))
// 				throw execveError(*this);
// 		}
// 	}
// 	this->_child_exited = true;
// }

void CgiHandler::finishChildProcess(void)
{
	int status;
	pid_t res = waitpid(this->_pid, &status, WNOHANG);

	if (res == 0)
		return ;
	else if (res == -1)
	{
		throw execveError(*this);
	}
	else
	{
		if (WIFEXITED(status))
		{
			if (WEXITSTATUS(status) != 0 || WIFSIGNALED(status))
				throw execveError(*this);
		}
	}
	this->_child_exited = true;
}

// note: tries to get content-type form request headers. if exists, use its value, otherwise keep empty
void CgiHandler::setEnvp(Client& client_obj)
{
	const HttpRequest &req = client_obj.getRequest();
	this->_requestBody = req.body;
	this->addEnvpElement("REQUEST_METHOD", client_obj.getRequest().method);
	this->addEnvpElement("SCRIPT_NAME", client_obj.getRequest().path);
	this->addEnvpElement("PATH_INFO", client_obj.getRequest().pathInfo);
	this->addEnvpElement("QUERY_STRING", client_obj.getRequest().query);
	this->addEnvpElement("CONTENT_LENGTH", std::to_string(client_obj.getRequest().contentLength));
	std::string contentType = "";
	std::unordered_map<std::string, std::string>::const_iterator it = req.headers.find("content-type");
	if (it != req.headers.end())
		contentType = it->second;
	this->addEnvpElement("CONTENT_TYPE", contentType);
	this->addEnvpElement("SERVER_PROTOCOL", client_obj.getRequest().version);

	this->addEnvpElement("GATEWAY_INTERFACE", "CGI/1.1");
	this->addEnvpElement("SERVER_SOFTWARE", "webserv/1.0");
	this->addEnvpElement("HTTP_HOST", "localhost");
	this->addEnvpElement("HTTP_USER_AGENT", "curl/7.88");
}

void CgiHandler::setArgsAndCgiPath(char* in_cgi_path)
{
	// if interpreterPath is empty / not set, use python path as default interpreter
	this->cgi_path = in_cgi_path;
	std::string cgi_interpreter = this->_interpreterPath.empty() ? "/usr/bin/python3" : this->_interpreterPath;

	if (access(this->cgi_path, R_OK) == -1) // new: should only need readable script. X_OK fails for .py files
	{
		throw fileAccessError(this->cgi_path);
	}

	std::string arg0 = cgi_interpreter;
	std::string arg1 = this->cgi_path;
	
	this->addArgsElement(arg0);
	this->addArgsElement(arg1);
}

void CgiHandler::printArgs(std::ostream& out) const
{
	int i = 0;
	
	out	<< "The number of arguments: " 
				<< this->_args_num << "\n";
	while (this->_args && this->_args[i] != NULL)
	{
		out	<< "The args[" << i << "]: "
					<< this->_args[i] << std::endl;
		i++;
	}		
}

void CgiHandler::printEnvp(std::ostream& out) const
{
	int i = 0;
	
	out	<< "The number of environment elements: " 
				<< this->_envp_num << "\n";
	while (this->_envp && this->_envp[i] != NULL)
	{
		out	<< "The envp[" << i << "]: "
			<< this->_envp[i] << std::endl;
		i++;
	}	
}

// void CgiHandler::closePipeFd(int opt)
// {
// 	if (opt == 1 || opt == 0)
// 	{
// 		close(this->_infd[0]);
// 		close(this->_outfd[1]);
// 	}

// 	if (opt == 2 || opt == 0)
// 	{
// 		close(this->_infd[1]);
// 		close(this->_outfd[0]);
// 	}
// }

// void CgiHandler::setNonBlockPipe(void)
// {
// 	fcntl(this->_outfd[0], F_SETFL, O_NONBLOCK);
// 	fcntl(this->_infd[1], F_SETFL, O_NONBLOCK);
// }

void CgiHandler::setNonBlockPipe(void)
{
	fcntl(this->_cgi_to_srv[0], F_SETFL, O_NONBLOCK);
	fcntl(this->_srv_to_cgi[1], F_SETFL, O_NONBLOCK);
}

const char* CgiHandler::getCgiPath(void) const
{
	return (this->cgi_path);
}

// int CgiHandler::getCgiInWriteFD(void) const
// {
// 	return (this->_infd[1]);
// }

int CgiHandler::getCgiInWriteFD(void) const
{
	return (this->_srv_to_cgi[1]);
}

// int CgiHandler::getCgiOutReadFD(void) const
// {
// 	return (this->_outfd[0]);
// }

int CgiHandler::getCgiOutReadFD(void) const
{
	return (this->_cgi_to_srv[0]);
}

/*
- checks if CGI request is fully done
1. updates child process status if needed (to avoid getting stuck)
2. checks completion conditions
*/
bool CgiHandler::IsCgiFinished(void)
{
	if (!this->_child_exited && this->_pid > 0) // to update child exited
	{
		try
		{
			this->finishChildProcess(); 
		}
		catch (...)
		{
			this->_child_exited = true; // avoids infinite wait loop
		}
	}
	if (this->_stdin_closed == true &&
		this->_stdout_closed == true &&
		this->_child_exited == true)
	{
		return (true);
	}

	return (false);
}

std::string CgiHandler::buildCgiResponse(void)
{
	//need to discuss the script will consist the whole http response
	// or the HTTPResponse serializer will do it
	return (this->_cgi_response);
}

// void CgiHandler::terminateChild(void)
// {
// 	if (this->_execution_child > 0)
// 	{
// 		kill(this->_execution_child, SIGTERM);
// 		this->closePipeFd(0);
// 	}
// }

void CgiHandler::terminateChild(void)
{
	if (this->_pid > 0)
	{
		kill(this->_pid, SIGTERM);
		this->closePipeFds(CLOSE_ALL);
	}
}


//merge
// check if input from server is valid (ex. file accessible) before moving on to executing (argv, envp)
bool CgiHandler::validateExecveArgs(char **argv, char **envp)
{
    if (!envp || !argv || !argv[0] || !argv[1])
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
Pipe 2: for server to read output from CGI (if fail, close first pipe for safety)
*/
bool CgiHandler::createPipes()
{
    if (pipe(_srv_to_cgi) == -1)
    {
        std::cerr << "CGI Error: pipe could not be created." << std::endl;
        return false;
    }
    if (pipe(_cgi_to_srv) == -1)
    {
		close(this->_srv_to_cgi[0]);
		close(this->_srv_to_cgi[1]);
		this->_srv_to_cgi[0] = -1;
		this->_srv_to_cgi[1] = -1;
        std::cerr << "CGI Error: pipe could not be created." << std::endl;
        return false;
    }
    return true;
}

// ft: close file descriptors of pipes accordingly (-1 for safety & prevent double-closing)
void CgiHandler::closePipeFds(PipeCloseCall action)
{
	if ((action == CLOSE_ALL || action == CLOSE_SRV_TO_CGI) && this->_srv_to_cgi[0] != -1)
	{
		close(this->_srv_to_cgi[0]);
		this->_srv_to_cgi[0] = -1;
	}
	if ((action == CLOSE_ALL || action == CLOSE_SRV_TO_CGI) && this->_srv_to_cgi[1] != -1)
	{
		close(this->_srv_to_cgi[1]);
		this->_srv_to_cgi[1] = -1;
	}
	if ((action == CLOSE_ALL || action == CLOSE_CGI_TO_SRV) && this->_cgi_to_srv[0] != -1)
	{
		close(this->_cgi_to_srv[0]);
		this->_cgi_to_srv[0] = -1;
	}
	if ((action == CLOSE_ALL || action == CLOSE_CGI_TO_SRV) && this->_cgi_to_srv[1] != -1)
	{
		close(this->_cgi_to_srv[1]);
		this->_cgi_to_srv[1] = -1;
	}
	if (action == CLOSE_READ_SRV_TO_CGI && this->_srv_to_cgi[0] != -1)
	{
		close(this->_srv_to_cgi[0]);
		this->_srv_to_cgi[0] = -1;
	}
	if (action == CLOSE_WRITE_CGI_TO_SRV && this->_cgi_to_srv[1] != -1)
	{
		close(this->_cgi_to_srv[1]);
		this->_cgi_to_srv[1] = -1;
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
    
    // redirect output: CGI writes to second pipe
    if (dup2(this->_cgi_to_srv[1], STDOUT_FILENO) == -1)
    {
        std::cerr << "CGI Error: dup2() failed." << std::endl;
        this->closePipeFds(CLOSE_ALL);
        return false;
    }
	this->closePipeFds(CLOSE_ALL);
    return true;
}

// ft: write POST(full) & GET(empty) request body to the first pipe shared w CGI (server -> CGI)
// ssize_t is signed (good for return values), size_t is unsigned
bool CgiHandler::writeRequestBodyToPipe()
{
	size_t writtenTotal = 0;
	while (writtenTotal < this->_requestBody.size())
	{
		ssize_t written = write(this->_srv_to_cgi[1],
			this->_requestBody.c_str() + writtenTotal,
			this->_requestBody.size() - writtenTotal);
		if (written < 0)
		{
			if (errno == EINTR) // if interrupted by signal, then retry, no failing
				continue;
			std::cerr << "CGI Error: write() failed." << std::endl;
			return false;
		}
		writtenTotal += static_cast<size_t>(written);
	}

    if (this->_srv_to_cgi[1] != -1) // states EOF (no waiting for more input)
	{
		close(this->_srv_to_cgi[1]);
		this->_srv_to_cgi[1] = -1;
	}
	this->_stdin_closed = true; // important for isCGIfinished ft
    return true;
}

// ft: read the CGI output (from the child process that executed the file script) from the second pipe (CGI -> server)
bool CgiHandler::readCgiOutputFromPipe()
{
    char buffer[4096];
    while (true)
    {
        ssize_t bytesRead = read(this->_cgi_to_srv[0], buffer, sizeof(buffer));
		if (bytesRead > 0)
		{
        	this->_cgiOutput.append(buffer, static_cast<size_t>(bytesRead));
			continue;
		}
		if (bytesRead == 0) // implies EOF
		{
			close(this->_cgi_to_srv[0]);
			this->_cgi_to_srv[0] = -1;
			this->_stdout_closed = true;
			return true;
		}
		if (errno == EINTR) // try again, else fail
			continue;
		std::cerr << "CGI Error: read() failed." << std::endl;
		return false;
    }
}

// ft: waits for child to finish or kill child if stuck too long 
// and captures the exit code, without blocking forever
// note: controlled waiting & timeout protection & final exit status
void CgiHandler::waitAndGetExitCode()
{
    int status = 0;
    time_t start = time(NULL);
    
    while (1)
    {
        pid_t result = waitpid(this->_pid, &status, WNOHANG);
        if (result > 0)
        {
            if (WIFEXITED(status))
                this->_exitCode = WEXITSTATUS(status);
            else if (WIFSIGNALED(status))
                this->_exitCode = 128 + WTERMSIG(status);
            this->_child_exited = true;
            break;
        }
		if (result == -1) // waitpid error
		{
			this->_exitCode = 1;
			this->_child_exited = true;
			break;
		}
        if (difftime(time(NULL), start) >= CGI_MAX_TIME)
        {
            kill(this->_pid, SIGKILL);
            waitpid(this->_pid, &status, 0);
            this->_exitCode = 124;
			this->_child_exited = true;
            std::cerr << "CGI Error: timeout reached." << std::endl;
            break;
        }
        usleep(100000); // sleeps to avoid busy CPU spin
    }
}

/*
MAIN execution function (called in handleCGI)
1. validate inputs (argv, envp)
2. fork () child
3. parent process (close unused pipes, write request body to child stdin, read full cgi output,
	from child stdout, wait for child/timemout, and get exit code
4. store result (output to response)
*/
bool CgiHandler::execute()
{
	if (!this->validateExecveArgs(this->_args, this->_envp))
	{
		this->closePipeFds(CLOSE_ALL);
		return false;
	}

    this->_pid = fork();
    if (this->_pid == -1)
    {
        this->closePipeFds(CLOSE_ALL);
        return false; 
    }
    
    if (this->_pid == 0)
    {
        if (!this->redirectIO())
            _exit(1);
        execve(this->_interpreterPath.c_str(), this->_args, this->_envp);
        std::cerr << RED << "CGI Error: execve() failed: "
					<< YELLOW << strerror(errno) 
					<< RED << "\nInterpreter path: " << this->_interpreterPath
					<< DEFAULT << std::endl;
        _exit(1);
    }

	this->closePipeFds(CLOSE_READ_SRV_TO_CGI);
	this->closePipeFds(CLOSE_WRITE_CGI_TO_SRV);
	if (!this->writeRequestBodyToPipe())
	{
		this->terminateChild();
		return false;
	}
	if (!this->readCgiOutputFromPipe())
	{
		this->terminateChild();
		return false;
	}
	this->waitAndGetExitCode();
	this->_cgi_response = this->_cgiOutput;
    return (this->_exitCode == 0);
}

void CgiHandler::setInterpreterPath(const std::string& i_path)
{
	this->_interpreterPath = i_path; 
}
