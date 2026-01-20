#ifndef ERREOR_HANDLER_HPP
# define ERREOR_HANDLER_HPP

# include <errno.h>
# include <unistd.h>
# include <string>
# include <iostream>
# include <sstream>
# include <cstring>

# define RED "\033[1;31m"
# define GREEN "\033[1;32m"
# define YELLOW "\033[1;33m"
# define BLUE "\033[1;34m"
# define MAGENTA "\033[1;35m"
# define CYAN "\033[1;36m"
# define DEFAULT "\033[0m"

class httpErrors
{
	private:
		std::string _reason;
		std::string _body;
		bool close_connection;
	
	public:
		httpErrors(void);
		httpErrors(const int status_code);
		httpErrors(const httpErrors& other);
		httpErrors& operator=(const httpErrors& other);
		~httpErrors(void);

		static const std::string reasonPhrase(int status_code);
		static const std::string errorBody(int status_code);

		// Client Errors
		void error400(void);
		void error403(void);
		void error404(void);
		void error405(void);
		void error408(void);
		void error413(void);
		void error414(void);

		// Server Errors
		void error500(void);
		void error502(void);
		void error504(void);
		void error505(void);

		const std::string& getReason(void) const;
		const std::string& getBody(void) const;
		const bool& getCloseConnection(void) const;
};

class errorHandler
{
	private:
		int _error_code;
		std::string _message;
		httpErrors _http_err_response;

		//std::string _reason;
		//std::string _body;

		errorHandler(void);
	
	public:
		errorHandler(const std::string& text);
		errorHandler(const int status_code);
		errorHandler(const errorHandler& other);
		errorHandler& operator=(const errorHandler& other);
		~errorHandler(void);

		void printStderr(void);
		void printStdout(void);
		void printFd(const int fd);

		int getErrorCode(void) const;
		
};

#endif