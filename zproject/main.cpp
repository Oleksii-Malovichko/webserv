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
		// const ServerConfig& s = servers[0];
		std::cout << "\nDEBUG:\n";
		for (size_t i = 0; i < servers.size(); i++)
		{
			const ServerConfig &s = servers[i];
			std::cout << "\nServer #" << i + 1 <<  ":" << std::endl; 
			std::cout << "ip = " << s.getIP() << std::endl;
			std::cout << "port = " << s.getPort() << std::endl;
			std::cout << "root = " << s.getRoot() << std::endl;
			std::cout << "index = " << s.getIndex() << std::endl;
			std::cout << "body_size = " << s.getClientMaxBodySize() << std::endl;
			std::cout << "404 page = " << s.getErrorPage(404) << std::endl;
			std::cout << "500 page = " << s.getErrorPage(500) << std::endl;

			const std::vector<LocationConfig> locations = s.getLocations();
			for (size_t i = 0; i < locations.size(); i++)
			{
				const LocationConfig &l = locations[i];
				std::cout << "\nLocation #" << i + 1 << ":" << std::endl;
				std::cout << "path = " << l.getPath() << std::endl;
				std::cout << "root = " << l.getRoot() << std::endl;
				std::vector<std::string> methods = l.getMethods();
				for (size_t i = 0; i < methods.size(); i++)
				{
					std::cout << "method #" << i << " = " << methods[i] << "; ";
				}
				std::cout << std::endl;
				std::cout << "upload = " << l.getUpload() << std::endl;
				std::cout << "upload_dir = " << l.getUploadDir() << std::endl;
				std::cout << "auto_index = " << l.getAutoIndex() << std::endl;
				std::cout << "index = " << l.getIndex() << std::endl;
				std::unordered_map<std::string, std::string> cgis = l.getCgi();
				for (auto it = cgis.begin(); it != cgis.end(); it++)
				{
					std::cout << "cgi extenstion = " << it->first << "; cgi bin = " << it->second << std::endl;
				}
			}
		}
		
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