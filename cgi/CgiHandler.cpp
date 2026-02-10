#include "CgiHandler.hpp"

CgiHandler::CgiHandler(void)
{
	this->_args_num = 0;
	this->_envp_num = 0;
	this->_envp = NULL;
	this->_args = NULL;
	
	if (pipe(this->_infd) == -1)
		throw pipeError("Failed to create input pipe");

	if (pipe(this->_outfd) == -1)
		throw pipeError("Failed to create output pipe");
	
}

CgiHandler* CgiHandler::_instance = nullptr;

CgiHandler::~CgiHandler(void)
{
	this->freeArgs();
	this->_args_num = 0;
	this->freeEnvp();
	this->_envp_num = 0;
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

std::string CgiHandler::runExecve(void)
{
	//the request budy string given to test,
	// later need to change from the http value
	std::string request_body = "test message for execve";
	
	char buffer[CGI_BUFFER_SIZE + 1];
	std::string response;
	int readbyte = 1;

	this->_execution_child = fork();
	if (this->_execution_child == -1 )
		throw forkError();
	
	if (_execution_child == 0)
	{
		if (dup2(this->_infd[0], STDIN_FILENO) == -1)
			throw dup2Error("Failed to dup2 _infd[0]");

		if (dup2(this->_outfd[1], STDOUT_FILENO) == -1)
			throw dup2Error("Failed to dup2 _infd[0]");

		this->closePipeFd(0);
		
		execve("/usr/bin/python3", 
			this->_args, this->_envp);
		throw execveError(*this);
		// perror("Failed to execute execve");
		// exit(1);
	}
	else
	{
		this->closePipeFd(1);
		// this->setNonBlockPipe();

		write(this->_infd[1], request_body.c_str(), request_body.length());
		close(this->_infd[1]);

		//This part need to continue -> write a nonblocking version
		/*
		while (readbyte > 0)
		{
			readbyte = read(this->_outfd[0], buffer, CGI_BUFFER_SIZE);
			if (readbyte < 0)
			{
				throw readError(*this);
			}
			if (readbyte > 0)
			{
				response.append(buffer, readbyte);
			}
		}
		


		waitpid(this->_execution_child, NULL, 0);
		*/
	
		/*
		Non-blocking version
		*/
		while (true)
		{
			readbyte = read(this->_outfd[0], buffer, CGI_BUFFER_SIZE);
			if (readbyte > 0)
			{
				response.append(buffer, readbyte);
			}
			else if (readbyte == 0)
			{
				close(this->_outfd[0]);
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
					close(this->_outfd[0]);
					throw readError(*this);
				}
			}
		}

		close(this->_outfd[0]);

		CgiHandler::_instance = this;
		signal(SIGALRM, cgi_timout_handler);
		alarm(CGI_MAX_TIME);

		int status;
		// waitpid(this->_execution_child, &status, WNOHANG);
		waitpid(this->_execution_child, &status, 0);

		if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
			throw execveError(*this);

		alarm(0);
	}

	return (response);
}

void CgiHandler::cgi_timout_handler(int) noexcept
{
	if (_instance)
	{
		_instance->closePipeFd(0);
		kill(_instance->_execution_child, SIGKILL);
	}
}

void CgiHandler::setEnvp(void)
{
	this->addEnvpElement("REQUEST_METHOD", "GET");
	this->addEnvpElement("SCRIPT_NAME", "/cgi-bin/test.py");
	this->addEnvpElement("PATH_INFO", "");
	this->addEnvpElement("QUERY_STRING", "name=Tom&age=25");
	this->addEnvpElement("CONTENT_LENGTH", "");
	this->addEnvpElement("CONTENT_TYPE", "");
	this->addEnvpElement("SERVER_PROTOCOL", "HTTP/1.1");

	this->addEnvpElement("GATEWAY_INTERFACE", "CGI/1.1");
	this->addEnvpElement("SERVER_SOFTWARE", "webserv/1.0");
	this->addEnvpElement("HTTP_HOST", "localhost");
	this->addEnvpElement("HTTP_USER_AGENT", "curl/7.88");
}

void CgiHandler::setArgsAndCgiPath(char* in_cgi_path)
{
	//later the following strings need to change the 
	// info which came from the request
	this->cgi_path = in_cgi_path;
	std::string cgi_interpreter = "/usr/bin/python3";

	if (access(this->cgi_path, X_OK) == -1)
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
	while (this->_args[i] != NULL)
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
	while (this->_envp[i] != NULL)
	{
		out	<< "The envp[" << i << "]: "
			<< this->_envp[i] << std::endl;
		i++;
	}	
}

void CgiHandler::closePipeFd(int opt)
{
	if (opt == 1 || opt == 0)
	{
		close(this->_infd[0]);
		close(this->_outfd[1]);
	}

	if (opt == 2 || opt == 0)
	{
		close(this->_infd[1]);
		close(this->_outfd[0]);
	}
}

void CgiHandler::setNonBlockPipe(void)
{
	fcntl(this->_outfd[0], F_SETFL, O_NONBLOCK);
	fcntl(this->_infd[1], F_SETFL, O_NONBLOCK);
}

const char* CgiHandler::getCgiPath(void) const
{
	return (this->cgi_path);
}

