#include "configParser.hpp"

// Constructors for httpMethods class
httpMethods::httpMethods(void)
{
	_on_get = false;
	_on_post = false;
	_on_put = false;
	_on_delete = false;
}

httpMethods::httpMethods(const 
	std::set<std::string>& methods):
	httpMethods()
{
	auto it_get = methods.find("GET");
	auto it_post = methods.find("POST");
	auto it_put = methods.find("PUT");
	auto it_delete = methods.find("DELETE");

	if (it_get != methods.end())
	{
		_on_get = true;
	}

	if (it_post != methods.end())
	{
		_on_post = true;
	}

	if (it_put != methods.end())
	{
		_on_put = true;
	}

	if (it_delete != methods.end())
	{
		_on_delete = true;
	}
}

httpMethods::httpMethods(const httpMethods& other)
{
	_on_get = other._on_get;
	_on_post = other._on_post;
	_on_put = other._on_put;
	_on_delete = other._on_delete;
}

httpMethods& httpMethods::operator=(const httpMethods& other)
{
	if (this != &other)
	{
		_on_get = other._on_get;
		_on_post = other._on_post;
		_on_put = other._on_put;
		_on_delete = other._on_delete;
	}
	return (*this);
}	

httpMethods::~httpMethods(void)
{

}


// Constructors for serverLocation class
serverLocation::serverLocation(void):
	_path(""), _root(""), _index(""),
	_methods(), _autoindex(false),
	_upload_on(false), _upload_store("")
{

}

/*
serverLocation::serverLocation(const std::string& in_path, 
		const std::string& in_root,
		const std::string &in_index,
		const std::set<std::string>& in_methods)
{
	_path = in_path;
	_root = in_root;
	_index = in_index;
	_methods = httpMethods(in_methods);
}

serverLocation::serverLocation(const bool in_autoindex,
		const bool in_upload_on,
		const std::string& in_upload_store)
{
	_autoindex = in_autoindex;
	_upload_on = in_upload_on;
	_upload_store = in_upload_store;
}
*/

serverLocation::serverLocation(const serverLocation& other)
{
	_path = other._path;
	_root = other._root;
	_index = other._index;
	_methods = other._methods;
	_autoindex = other._autoindex;
	_upload_on = other._upload_on;
	_upload_store = other._upload_store;
}

serverLocation& serverLocation::operator=(const serverLocation& other)
{
	if (this != &other)
	{
		_path = other._path;
		_root = other._root;
		_index = other._index;
		_methods = other._methods;
		_autoindex = other._autoindex;
		_upload_on = other._upload_on;
		_upload_store = other._upload_store;
	}
	return (*this);
}

serverLocation::~serverLocation(void)
{

}


// Constructors for serverInfo class
serverInfo::serverInfo(void):
	_locations(), _port(80),
	_server_name(""), _max_body_size(1000000),
	_error_pages()
{

}

serverInfo::serverInfo(const serverInfo& other)
{
	_locations = other._locations;
	_port = other._port;
	_server_name = other._server_name;
	_max_body_size = other._max_body_size;
	_error_pages = other._error_pages;
}

serverInfo& serverInfo::operator=(const serverInfo& other)
{
	if (this != &other)
	{
		_locations = other._locations;
		_port = other._port;
		_server_name = other._server_name;
		_max_body_size = other._max_body_size;
		_error_pages = other._error_pages;
	}
	return (*this);
}

serverInfo::~serverInfo(void)
{

}

void serverInfo::parseServerBlock(
	const std::string& serverblock)
{
	// To be implemented: parsing logic for server block
	(void)serverblock; // Placeholder to avoid unused parameter warning
}


// Constructors for configParser class
configParser::configParser(void)
{
	_servers = std::vector<serverInfo>();
}

configParser::~configParser(void)
{

}

int configParser::checkInputArguments(
	int argc, char** argv)
{
	std::string config_end = "_config";

	
	if (argc == 1)
	{
		std::cout 	<< YELLOW << "Default configfile applied: "
					<< BLUE << "../configuration_files"
					<< "/default_config" 
					<<DEFAULT << std::endl;
		return (1);
	}
	else if (argc == 2)
	{
		std::string path = std::string(argv[1]);
		size_t it_config = path.find(config_end);
		if(it_config != std::string::npos && 
			it_config + config_end.length() == path.length())
		{
			return (readConfigFile(path));
		}
		else
		{
			std::cerr 	<< RED << "The given configuration file: "
						<< YELLOW << path << RED 
						<< " is not ending with " 
						<< BLUE << config_end
						<< DEFAULT << std::endl;
			return (-1);
		}
	}
	else
	{
		std::cerr 	<< RED << "The number of input arguments "
					<< "are not correct!\n"
					<< YELLOW << "Please use the following format:\n"
					<< BLUE << "./webserv [configuration file ending with config]"
					<< DEFAULT << std::endl;
		return (-1);
	}
}

void configParser::addServer(
	const std::string& line)
{
	std::string server = "server{";
	std::string substring = "";
	size_t server_pos = 0;
	size_t next_server_pos = 0;

	while ((server_pos = line.find(
		server, server_pos)) != std::string::npos)
	{
		// Parse server block
		serverInfo new_server;
		server_pos += server.length();
		next_server_pos = line.find(
			server, server_pos);
		
		substring = line.substr(
				server_pos, next_server_pos - server_pos);
		
		new_server.parseServerBlock(substring);
		_servers.push_back(new_server);
	}
}

int configParser::readConfigFile(
	const std::string& path)
{
	int readbyte = 1;
	char buffer[BUFFER_SIZE + 1];
	std::string line = "";
	size_t count_line = 0;

	int config_fd = open(path.c_str(), O_RDONLY);
	if (config_fd == -1)
	{
		std::cerr 	<< RED << "Failed to open config file: "
					<< YELLOW << path 
					<< DEFAULT << std::endl;
		return (-1);
	}

	std::cout << GREEN << "Config file: "
			  << BLUE << path 
			  << DEFAULT << std::endl;
	
	while (readbyte > 0)
	{
		readbyte = read(config_fd, buffer, BUFFER_SIZE);
		if (readbyte == -1)
		{
			std::cerr 	<< RED << "Failed to read config file: "
						<< YELLOW << path 
						<< DEFAULT << std::endl;
			close(config_fd);
			return (-1);
		}
		else if (readbyte == 0)
		{
			break;
		}

		buffer[readbyte] = '\0';

		count_line++;
		line = std::string(buffer);

		std::cout	<< BLUE << "The " << count_line 
					<< " line of the config file is:\n" 
					<< MAGENTA << line << DEFAULT << std::endl;
	}
	close(config_fd);
	return (0);
}