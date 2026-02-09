#ifndef CGIEXCEPTIONS_HPP
# define CGIEXCEPTIONS_HPP

# include <exception>
# include <string>
# include <cstring>
# include <sstream>
# include <stdlib.h>

# define RED "\033[1;31m"
# define GREEN "\033[1;32m"
# define YELLOW "\033[1;33m"
# define BLUE "\033[1;34m"
# define MAGENTA "\033[1;35m"
# define CYAN "\033[1;36m"
# define DEFAULT "\033[0m"

// # include "CgiHandler.hpp"
class CgiHandler;

class Exceptions: public std::exception
{
	private:
		std::string _message;

	public:
		void setMessage(const std::string& inputMessage);
		const char* what() const throw();

};

class pipeError: public Exceptions
{
	public:
		pipeError(
			const std::string& err_message);
};

class forkError: public Exceptions
{
	public:
		forkError(void);
};

class dup2Error: public Exceptions
{
	public:
		dup2Error(const std::string& err_message);
};

class execveError: public Exceptions
{
	public:
		execveError(CgiHandler& cgi_hand);
};

class readError: public Exceptions
{
	public:
		readError(CgiHandler& cgi_hand);
};

#endif