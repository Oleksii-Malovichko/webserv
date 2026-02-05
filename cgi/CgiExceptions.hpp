#ifndef CGIEXCEPTIONS_HPP
# define CGIEXCEPTIONS_HPP

# include <exception>
# include <string>

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

#endif