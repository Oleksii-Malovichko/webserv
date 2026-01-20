# include "errorHandler.hpp"

// httpError Client error functions
void httpErrors::error400(void)
{
	this->_reason = "Bad Request";
	this->_body = "<html><body><h1>"
			"400 Bad Request</h1></body></html>";
	this->close_connection = true;
}

void httpErrors::error403(void)
{
	this->_reason = "Forbidden";
	this->_body = "<html><body><h1>"
			"403 Forbidden</h1></body></html>";
	this->close_connection = false;
}

void httpErrors::error404(void)
{
	this->_reason = "Not Found";
	this->_body = "<html><body><h1>"
			"404 Not Found</h1></body></html>";
	this->close_connection = false;
}

void httpErrors::error405(void)
{
	this->_reason = "Method Not Allowed";
	this->_body = "<html><body><h1>"
			"405 Method Not Allowed</h1></body></html>";
	this->close_connection = false;
}

void httpErrors::error408(void)
{
	this->_reason = "Request Timeout";
	this->_body = "<html><body><h1>"
			"408 Request Timeout</h1></body></html>";
	this->close_connection = true;
}

void httpErrors::error413(void)
{
	this->_reason = "Payload Too Large";
	this->_body = "<html><body><h1>"
			"413 Payload Too Large</h1></body></html>";
	this->close_connection = true;
}

void httpErrors::error414(void)
{
	this->_reason = "URI Too Long";
	this->_body = "<html><body><h1>"
			"414 URI Too Long</h1></body></html>";
	this->close_connection = true;
}

// httpError Server error functions
void httpErrors::error500(void)
{
	this->_reason = "Internal Server Error";
	this->_body = "<html><body><h1>"
			"500 Internal Server Error</h1></body></html>";
	this->close_connection = false;
}

void httpErrors::error502(void)
{
	this->_reason = "Bad Gateway";
	this->_body = "<html><body><h1>"
			"502 Bad Gateway</h1></body></html>";
	this->close_connection = false;
}

void httpErrors::error504(void)
{
	this->_reason = "Gateway Timeout";
	this->_body = "<html><body><h1>"
			"504 Gateway Timeout</h1></body></html>";
	this->close_connection = false;
}

void httpErrors::error505(void)
{
	this->_reason = "HTTP Version Not Supported";
	this->_body = "<html><body><h1>"
			"505 HTTP Version Not Supported</h1></body></html>";
	this->close_connection = false;
}

// Constructors fot httpErrors class
httpErrors::httpErrors(void):
	 _reason(""), _body(""), close_connection(false)
{

}

httpErrors::httpErrors(const httpErrors& other):
	_reason(other._reason), _body(other._body), close_connection(other.close_connection)
{

}

httpErrors& httpErrors::operator=(const httpErrors& other)
{
	if (this != &other)
	{
		this->_reason = other._reason;
		this->_body = other._body;
		this->close_connection = other.close_connection;
	}

	return (*this);
}

httpErrors::httpErrors(const int status_code)
{
	switch (status_code)
	{
	case 400:
		this->error400();
		break;
	case 403:
		this->error403();
		break;
	case 404:
		this->error404();
		break;
	case 405:
		this->error405();
		break;
	case 408:
		this->error408();
		break;
	case 413:
		this->error413();
		break;
	case 414:
		this->error414();
		break;
	case 500:
		this->error500();
		break;
	case 502:
		this->error502();
		break;
	case 504:
		this->error504();
		break;
	case 505:
		this->error505();
		break;
	
	default:
		std::cerr	<< YELLOW << "Undefined error code"
					<< DEFAULT << std::endl;
		break;
	}
}

httpErrors::~httpErrors(void)
{

}

// getters for httpErrors class
const std::string& httpErrors::
	getReason(void) const
{
	return (this->_reason);
}

const std::string& httpErrors::
	getBody(void) const
{
	return (this->_body);
}

const bool& httpErrors::
	getCloseConnection(void) const
{
	return (this->close_connection);
}

// Concstructors for errorHandler class
errorHandler::errorHandler(void): _error_code(0), _message(""),
	 _http_err_response()
{

}

errorHandler::errorHandler(const std::string& text): _error_code(errno), 
	_message(text), _http_err_response()
{

}

errorHandler::errorHandler(const int status_code)
{
	std::stringstream ss;
	
	this->_http_err_response = httpErrors(status_code);
	this->_error_code = errno;

	ss	<< RED << "HTTP Error occured:\n" 
		<< YELLOW << "errno code: " << this->_error_code << "\n"
		<< BLUE << "Status code: " << status_code << "\n"
		<< "Reason: " << this->_http_err_response.getReason() << "\n"
		<< "Body: " << this->_http_err_response.getBody()
		<< DEFAULT << std::endl;

	this->_message = ss.str();
}

errorHandler::errorHandler(const errorHandler& other): 
	_error_code(other._error_code), _message(other._message),
	_http_err_response(other._http_err_response)
{

}

errorHandler& errorHandler::operator=(const errorHandler& other)
{
	if (this != &other)
	{
		this->_error_code = other._error_code;
		this->_message = other._message;
		this->_http_err_response = other._http_err_response;
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

void errorHandler::printFd(const int fd)
{
	write(fd, this->_message.c_str(), 
		this->_message.size());

	this->_error_code = 0;
}

int errorHandler::getErrorCode(void) const
{
	return (this->_error_code);
}
