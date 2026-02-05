#include "CgiHandler.hpp"

CgiHandler::CgiHandler(void)
{
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

