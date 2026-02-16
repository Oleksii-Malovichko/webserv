#include "CgiExceptions.hpp"
#include "CgiHandler.hpp"

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

dup2Error::dup2Error(const std::string& err_message)
{
	std::stringstream ss;

	ss	<< RED << err_message << ": "
		<< strerror(errno)
		<< DEFAULT << std::endl;

	setMessage(ss.str());
}

/*
This function need to close the pipefd-s and the exit the child process
Here it need use _exit because simple exit runs the 
class destructors and flushes stdio
*/
execveError::execveError(CgiHandler& cgi_hand)
{
	std::stringstream ss;

	ss	<< "The execve with the following inputs failed:\n"
		<< "The cgi path: " << cgi_hand.getCgiPath();
		
	cgi_hand.printArgs(ss);
	cgi_hand.printEnvp(ss);
		
	ss << "The errno message: " << strerror(errno);

	setMessage(ss.str());

	cgi_hand.closePipeFd(0);
	_exit(1);
}

readError::readError(CgiHandler& cgi_hand)
{
	std::stringstream ss;

	ss << "Failed to read CGI STDOUT: "
		<< strerror(errno);

	setMessage(ss.str());
	cgi_hand.closePipeFd(0);
}

fileAccessError::fileAccessError(const std::string& file_path)
{
	std::stringstream ss;

	ss << "Can't access the executable file: "
		<< file_path
		<< " Error code: " << strerror(errno);

	setMessage(ss.str());
}

writeError::writeError(CgiHandler& cgi_hand)
{
	std::stringstream ss;

	ss << "Failed to write CGI STDIN: "
		<< strerror(errno);

	setMessage(ss.str());
	cgi_hand.closePipeFd(0);
}
