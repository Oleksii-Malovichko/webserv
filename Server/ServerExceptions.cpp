#include "ServerExceptions.hpp"

void ServerExceptions::setMessage(
	const std::string& inputMessage)
{
	this->_message = inputMessage;
}

const char* ServerExceptions::what() const throw()
{
	return (this->_message.c_str());
}

DirectoryListingOffError::DirectoryListingOffError(void)
{
	std::stringstream ss;

	ss	<< "Directory Listing disabled" 
		<< std::endl;

	setMessage(ss.str());
}
