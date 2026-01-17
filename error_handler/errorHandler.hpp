#ifndef ERREOR_HANDLER_HPP
# define ERREOR_HANDLER_HPP

# include <errno.h>
# include <string>
# include <iostream>
# include <cstring>

# define RED "\033[1;31m"
# define GREEN "\033[1;32m"
# define YELLOW "\033[1;33m"
# define BLUE "\033[1;34m"
# define MAGENTA "\033[1;35m"
# define CYAN "\033[1;36m"
# define DEFAULT "\033[0m"

class errorHandler
{
	private:
		int _error_code;
		std::string _message;

		errorHandler(void);
	
	public:
		errorHandler(const std::string& text);
		errorHandler(const errorHandler& other);
		errorHandler& operator=(const errorHandler& other);
		~errorHandler(void);

		void printStderr(void);
		void printStdout(void);

		int getErrorCode(void) const;
		
};

#endif