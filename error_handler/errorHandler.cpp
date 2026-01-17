# include "errorHandler.hpp"

// Concstructors
errorHandler::errorHandler(void) : _error_code(0), _message("")
{

}

errorHandler::errorHandler(const std::string& text) : _error_code(errno), _message(text)
{

}

errorHandler::errorHandler(const errorHandler& other): 
	_error_code(other._error_code), _message(other._message)
{

}

errorHandler& errorHandler::operator=(const errorHandler& other)
{
	if (this != &other)
	{
		this->_error_code = other._error_code;
		this->_message = other._message;
	}
	return (*this);
}

errorHandler::~errorHandler(void)
{

}

// Public member functions
void errorHandler::printStderr(void)
{
	std::cerr	<< RED << this->_message << ": " << strerror(this->_error_code) 
				<< DEFAULT << std::endl;

	this->_error_code = 0;
}

void errorHandler::printStdout(void)
{
	std::cout	<< MAGENTA << this->_message << ": " << strerror(this->_error_code) 
				<< DEFAULT << std::endl;

	this->_error_code = 0;
}

int errorHandler::getErrorCode(void) const
{
	return (this->_error_code);
}
