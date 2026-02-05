#include "CgiExceptions.hpp"

//Error classes
void Exceptions::setMessage(
	const std::string& inputMessage)
{
	this->_message = inputMessage;
}

const char* Exceptions::what() const throw()
{
	return (this->_message.c_str());
}


pipeError::pipeError(
	const std::string& err_message)
{
	std::stringstream ss;

	ss	<< RED << err_message << ": "
		<< strerror(errno)
		<< DEFAULT << std::endl;

	setMessage(ss.str());
}

forkError::forkError()
{
	std::stringstream ss;

	ss << "Failed to create child process: "
		<< strerror(errno);

	setMessage(ss.str());
}
