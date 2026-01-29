#include "Server.hpp"
#include "ConfigParser.hpp"
#include <csignal>

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		std::cerr << "Your progam must use a configuration file" << std::endl;
		return 1;
	}
	// std::cout << "Server pid: " << getpid() << std::endl;
	try
	{
		ConfigParser parser(argv[1]);
		const std::vector<ServerConfig>& servers = parser.getWebServ().getServers();
		if (servers.empty())
		{
			std::cerr << "No servers found in config" << std::endl;
			return 1;
		}
		const ServerConfig& s = servers[0];
		std::cout << "port = " << s.getPort() << std::endl;
		std::cout << "root = " << s.getRoot() << std::endl;
		std::cout << "index = " << s.getIndex() << std::endl;
		std::cout << "body_size = " << s.getClientMaxBodySize() << std::endl;
		std::cout << "404 page = " << s.getErrorPage(404) << std::endl;
		std::cout << "500 page = " << s.getErrorPage(500) << std::endl;
		
		// Server server(static_cast<const std::string>(argv[1]));
		// server.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Fatal error: " << e.what() << std::endl;
		return 1;
	}
	// std::cout << "Server stopped" << std::endl;
	return 0;
}

// google-chrome --new-tab ../en.subject.pdf 