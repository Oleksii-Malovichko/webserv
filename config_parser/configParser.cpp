#include "configParser.hpp"

/* 
Skip whitespace characters in a string
starting from start_pos 
*/

size_t skipWhitespace(
	const std::string& str, size_t start_pos)
{
	size_t pos = start_pos;
	while (pos < str.length() && 
			(str[pos] == ' ' || str[pos] == '\t' 
				|| str[pos] == '\n' || str[pos] == '\r'
				|| str[pos] == '\v' || str[pos] == '\f'))
	{
		pos++;
	}
	return (pos);
}

size_t skipWordAndWhitespace(
	const std::string& str,
	const std::string& search,
	size_t start_pos)
{
	size_t pos = str.find(search);
	std::cout << YELLOW <<"pos befor while: " << pos
				<< DEFAULT << std::endl;
	size_t save_pos = pos;
	while (pos != std::string::npos && pos < start_pos)
	{
		
		save_pos = pos;
		pos = str.find(search, pos + 1);

		std::cout << YELLOW <<"pos in while: " << pos
				<< " save_pos in while: " << save_pos
				<< DEFAULT << std::endl;
	}

	std::cout << "in skipWordAndWhitespace, save_pos: " 
			  << save_pos 
			  << " str char: f" 
			  <<  str[save_pos] << "f" 
			  << "pos: " << pos
			  << std::endl;
	
	if (save_pos != std::string::npos)
	{
		save_pos += search.length();
		save_pos = skipWhitespace(str, save_pos);
	}

	std::cout << "The save_pos " << save_pos
		<< std::endl;

	return (save_pos);
}

std::string getInfo(
	const std::string& str,
	size_t start_pos, const char delimiter)
{
	size_t end_pos = start_pos;
	std::string info_result = "";

	while (end_pos < str.length() && 
		str[end_pos] != delimiter)
	{
		end_pos++;
	}

	info_result = str.substr(start_pos, end_pos - start_pos);
	return (info_result);
}


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

// Getters for httpMethods class
bool httpMethods::getGet(void) const
{
	return (_on_get);
}

bool httpMethods::getPost(void) const
{
	return (_on_post);
}

bool httpMethods::getPut(void) const
{
	return (_on_put);
}

bool httpMethods::getDelete(void) const
{
	return (_on_delete);
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

// Add location block
void serverLocation::addLocation(
	std::string& locationblock)
{
	// Parse root
	std::string root_n = "root";
	size_t root_n_pos = 0;
	root_n_pos = skipWordAndWhitespace(
		locationblock, root_n, root_n_pos);
	if (root_n_pos != std::string::npos)
	{
		_root = getInfo(
			locationblock, root_n_pos, ';');
	}

	// Parse index
	std::string index_n = "index";
	size_t index_n_pos = 0;
	index_n_pos = skipWordAndWhitespace(
		locationblock, index_n, root_n_pos);
	if (index_n_pos != std::string::npos)
	{
		_index = getInfo(
			locationblock, index_n_pos, ';');	
	}

	// Parse methods
	std::string methods_n = "methods";
	size_t methods_n_pos = 0;
	methods_n_pos = skipWordAndWhitespace(
		locationblock, methods_n, methods_n_pos);
	if (methods_n_pos != std::string::npos)
	{
		std::set<std::string> set_methods;
		std::string methods_str = getInfo(
			locationblock, methods_n_pos, ';');
		size_t start_pos = 0;
		while (start_pos < methods_str.length())
		{
			size_t end_pos = start_pos;
			while (end_pos < methods_str.length() &&
				(methods_str[end_pos] != ' ' &&
					methods_str[end_pos] != ';'))
			{
				end_pos++;
			}
			std::string method_substr = methods_str.substr(
				start_pos, end_pos - start_pos);
			set_methods.insert(method_substr);
			start_pos = end_pos + 1;
		}
		_methods = httpMethods(set_methods);
	}
	
	// Parse upload
	std::string upload_n = "upload";
	size_t upload_n_pos = 0;
	upload_n_pos = skipWordAndWhitespace(
		locationblock, upload_n, index_n_pos);
	if (upload_n_pos != std::string::npos)
	{
		std::string upload_str = getInfo(
			locationblock, upload_n_pos, ';');
		if (upload_str == "on")
		{
			_upload_on = true;
		}
		else if (upload_str == "off")
		{
			_upload_on = false;
		}
	}

	// Parse upload store
	std::string upload_store_n = "upload_store";
	size_t upload_store_n_pos = 0;
	upload_store_n_pos = skipWordAndWhitespace(
		locationblock, upload_store_n, upload_n_pos);
	if (upload_store_n_pos != std::string::npos)
	{
		_upload_store = getInfo(
			locationblock, upload_store_n_pos, ';');
	}

	// Parse autoindex
	std::string autoindex_n = "autoindex";
	size_t autoindex_n_pos = 0;
	autoindex_n_pos = skipWordAndWhitespace(
		locationblock, autoindex_n, upload_store_n_pos);
	if (autoindex_n_pos != std::string::npos)
	{
		std::string autoindex_str = getInfo(
			locationblock, autoindex_n_pos, ';');
		if (autoindex_str == "on")
		{
			_autoindex = true;
		}
		else if (autoindex_str == "off")
		{
			_autoindex = false;
		}
	}
}

// Getters for serverLocation class
const std::string& serverLocation::getPath(void) const
{
	return (_path);
}

const std::string& serverLocation::getRoot(void) const
{
	return (_root);
}

const std::string& serverLocation::getIndex(void) const
{
	return (_index);
}

const httpMethods& serverLocation::getMethods(void) const
{
	return (_methods);
}

bool serverLocation::getAutoindex(void) const
{
	return (_autoindex);
}

bool serverLocation::getUploadOn(void) const
{
	return (_upload_on);
}

const std::string& serverLocation::
	getUploadStore(void) const
{
	return (_upload_store);
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
	const std::string& serverblock, size_t start_server_pos)
{
	// Parse port
	std::size_t port_startpos = start_server_pos;
	port_startpos = 
		skipWordAndWhitespace(serverblock, "listen", port_startpos);
	size_t port_endpos = port_startpos;
	while (port_endpos < serverblock.length() &&
		isdigit(serverblock[port_endpos]))
	{
		port_endpos++;
	}

	std::string port_str = serverblock.substr(
		port_startpos, port_endpos - port_startpos);
	try
	{
		_port = std::stoi(port_str);
	}
	catch(const std::exception& e)
	{
		std::cerr	<< RED << "Failed to make integer from '" 
					<< YELLOW << port_str << RED
					<< "' in parseServerBlock: "
					<< e.what() << DEFAULT << std::endl;
	}

	std::cout << CYAN << "The port_str: " << port_str
				<< "\nThe seerverblock[port_endpos] "
				<< serverblock[port_endpos]
				<< DEFAULT << std::endl;

	
	//parse server_name
	std::string server_n = "server_name";
	size_t server_n_pos = 
		skipWordAndWhitespace(serverblock, server_n,
			port_endpos);
	if (server_n_pos != std::string::npos)
	{
		_server_name = getInfo(
			serverblock, server_n_pos, ';');
	}

	std::cout << BLUE << "The server_name: "
		<< _server_name 
		<< "\nserver_n_pos: " << server_n_pos
		<< "\nport_endpos: " << port_endpos
		<< DEFAULT << std::endl;

	
	// parse root
	std::string root_str = "root";
	size_t root_pos = skipWordAndWhitespace(
		serverblock, root_str, server_n_pos);
	if (root_pos != std::string::npos)
	{
		_s_root = getInfo(
			serverblock, root_pos, ';');

		std::cout << CYAN << "The _s_root: "
					<< _s_root << DEFAULT << std::endl;
	}

	// parse index
	std::string index_str = "index";
	size_t index_pos = skipWordAndWhitespace(
		serverblock, index_str, root_pos);
	if (index_pos != std::string::npos)
	{
		_s_index = getInfo(serverblock, 
			index_pos, ';');

		std::cout << CYAN << "The _s_index: "
					<< _s_index << DEFAULT << std::endl;
	}

	// parse max_body_size
	std::string max_body_str = "client_max_body_size";
	size_t max_body_pos = skipWordAndWhitespace(
		serverblock, max_body_str, index_pos);
	if (max_body_pos != std::string::npos)
	{
		std::string max_body_size_str = getInfo(
			serverblock, max_body_pos, ';');

		std::cout << YELLOW << "The max_body_size_str: "
					<< max_body_size_str
					<< DEFAULT << std::endl;
		_max_body_size = std::stoul(max_body_size_str);
	}
	
	/*
	// parse locations
	std::string location_str = "location";
	std::string substring = "";
	size_t location_pos = 0;
	size_t next_location_pos = 0;
	
	while ((location_pos = serverblock.find(
		location_str, location_pos)) != std::string::npos)
	{
		serverLocation new_location;
		location_pos += location_str.length();
		next_location_pos = serverblock.find(
			location_str, location_pos);
		
		substring = serverblock.substr(
				location_pos, next_location_pos - location_pos);

		new_location.addLocation(substring);
	}

	*/

	// parse error pages
	std::string error_page_str = "error_page";
	size_t error_page_pos = 0;
	while ((error_page_pos = skipWordAndWhitespace(serverblock,
		error_page_str, error_page_pos )) != std::string::npos)
	{
		std::string error_code = getInfo(
			serverblock, error_page_pos, ' ');
		int error_code_int = std::stoi(error_code);
		std::string error_path = getInfo(
			serverblock, error_page_pos + 
			error_code.length(), ';');

		std::cout << "ERROR CODE: " << error_code_int
					<< " Error_path: " << error_path << std::endl;
		_error_pages[error_code_int] = error_path;

		size_t semicolon_pos = serverblock.find(';', error_page_pos);
		if (semicolon_pos != std::string::npos)
		{
			error_page_pos = semicolon_pos + 1;
		}
		else
		{
			break;
		}

		std::cout << "The semicolon pos: " << semicolon_pos
					<< "The error page pos: " << error_page_pos
					<< std::endl;
	}
}

// Getters for serverInfo class
int serverInfo::getPort(void) const
{
	return (_port);
}

const std::string& serverInfo::getServerName(void) const
{
	return (_server_name);
}

const std::string& serverInfo::getRoot(void) const
{
	return (_s_root);
}

const std::string& serverInfo::getIndex(void) const
{
	return (_s_index);
}

size_t serverInfo::getMaxBodySize(void) const
{
	return (_max_body_size);
}

const std::map<int, std::string>& serverInfo::getErrorPages(void) const
{
	return (_error_pages);
}

const std::vector<serverLocation>& serverInfo::getLocations(void) const
{
	return (_locations);
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
	std::string default_path = 
			"../configuration_files/default_config";
	
	if (argc == 1)
	{
		std::cout 	<< YELLOW << "Default configfile applied: "
					<< BLUE << "../configuration_files"
					<< "/default_config" 
					<<DEFAULT << std::endl;
		
		return(readConfigFile(default_path));
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
		
		new_server.parseServerBlock(substring, server_pos);
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
	
		addServer(line);
	}
	close(config_fd);
	return (0);
}

void configParser::printServersInfo(void) const
{ 
	std::stringstream ss;
	
	for (size_t i = 0; i < _servers.size(); i++)
	{
		ss << GREEN <<"Port: " << _servers[i].getPort() << "\n"
		   << "Server name: " << _servers[i].getServerName() << "\n"
		   << "Server Root: " << _servers[i].getRoot() << "\n"
		   << "Server Index: " << _servers[i].getIndex() << "\n"
		   << "Max body size: " << _servers[i].getMaxBodySize() << "\n";
		
		const std::map<int, std::string>& errors = 
			_servers[i].getErrorPages();
		for (std::map<int, std::string>::const_iterator it =
			errors.begin(); it != errors.end(); ++it)
		{
			ss	<< "Error Code: " << it->first
				<< " Error path: " << it->second << "\n";
		}
		
		const std::vector<serverLocation>& locations = 
			_servers[i].getLocations();
		for (size_t j = 0; j < locations.size(); j++)
		{
			ss << std::boolalpha;
			ss << CYAN << "\nLocation Path: " 
			   << locations[j].getPath() << "\n"
			   << "Location Root: " 
			   << locations[j].getRoot() << "\n"
			   << "Location Index: " 
			   << locations[j].getIndex() << "\n"
			   << "Location Methods:\n"
			   << "GET: " << locations[j].getMethods().getGet() << "\n"
			   << "POST: " << locations[j].getMethods().getPost() << "\n"
			   << "PUT: " << locations[j].getMethods().getPut() << "\n"
			   << "DELETE: " << locations[j].getMethods().getDelete() << "\n"
			   << "Autoindex: " << locations[j].getAutoindex() << "\n"
			   << "Upload: " << locations[j].getUploadOn() << "\n"
			   << "Upload Store: "
			   << locations[j].getUploadStore() << "\n"
			   << DEFAULT;
		}
	}

	std::cout << GREEN
			  << "\nTotal number of servers: " 
			  << _servers.size() << "\n"
			  << DEFAULT << std::endl;
	std::cout << ss.str();
}