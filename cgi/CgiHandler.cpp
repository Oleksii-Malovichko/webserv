#include "CgiHandler.hpp"

CgiHandler::CgiHandler(void)
{
	this->_args_num = 0;
	this->_envp_num = 0;
	
	if (pipe(this->_infd) == -1)
		throw pipeError("Failed to create input pipe");

	if (pipe(this->_outfd) == -1)
		throw pipeError("Failed to create output pipe");

	this->_execution_child = fork();
	if (this->_execution_child == -1 )
		throw forkError();
	
	this->_execution_start_time = 
		std::chrono::steady_clock::now();
}

CgiHandler::~CgiHandler(void)
{
	
}

int CgiHandler::addArgsElement(std::string& value)
{
	this->_args_num++;
	int i = 0;

	char **new_args = new char*[this->_args_num + 1];
	while (i < this->_args_num - 1)
	{
		new_args[i] = strdup(this->_args[i]);
		i++;
	}
	
	new_args[i] = strdup(value.c_str());
	new_args[this->_args_num] = NULL;
	delete[] this->_args;
	this->_args = new_args;
}

int CgiHandler::addEnvpElement(std::string& key,
	std::string& value)
{
	std::string add_str = key + "=" + value;
	this->_envp_num++;
	int i = 0;

	char **new_envp = new char*[this->_envp_num + 1];
	while (i < this->_envp_num - 1)
	{
		new_envp[i] = strdup(this->_envp[i]);
		i++;
	}

	new_envp[i] = strdup(add_str.c_str());
	new_envp[this->_envp_num] = NULL;
	delete[] this->_envp;
	this->_envp = new_envp;
}	

int CgiHandler::runExecve(void) const
{
	//the request budy string given to test,
	// later need to change from the http value
	std::string request_body = "test body for execve";
	
	char buffer[CGI_BUFFER_SIZE + 1];
	std::string response;
	int readbyte = 1;
	
	if (_execution_child == 0)
	{
		execve(this->cgi_path, 
			this->_args, this->_envp);
	}
	else
	{
		write(this->_infd[1], request_body.c_str(), request_body.length());
		readbyte = read(this->_outfd[0], buffer, CGI_BUFFER_SIZE);
		if (readbyte > 0)
		{
			response += std::string(buffer);
		}
	}
}

