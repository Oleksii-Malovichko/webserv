#include "CgiHandler.hpp"

// c++ -Wall -Werror -Wextra -fsanitize=address -g -std=c++17 CgiExceptions.cpp CgiHandler.cpp cgi_test.cpp -o cgi_test
int main(int argc, char **argv)
{
	std::string cgi_http_response;
	
	if (argc != 2)
	{
		std::cerr	<< RED << "Please give the input script file."
					<< DEFAULT << std::endl;
		return (1);
	}

	try
	{
		CgiHandler cgi_obj;
		cgi_obj.setArgsAndCgiPath(argv[1]);
		cgi_obj.setEnvp();
		cgi_http_response = cgi_obj.runExecve();

		std::cout	<< GREEN << "The HTTP response: "
					<< CYAN << cgi_http_response
					<< DEFAULT << std::endl;
	}
	catch(const std::exception& e)
	{
		std::cerr	<< RED 
					<< "The following error occured " 
					<< e.what() << '\n';
	}
	catch(...)
	{
		std::cerr << YELLOW << "Some error"; 
	}

	return (0);
}