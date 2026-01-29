#include "ConfigParser.hpp"

const WebServConfig &ConfigParser::getWebServ() const
{
	return this->webserv;
}

void ConfigParser::parseServerLine(const std::string &line, ServerConfig &currentServer)
{
	std::vector<std::string> tokens;
	std::string newLine;
	if (line.find("listen ") == 0)
	{
		newLine = removeComment(line);
		// std::cout << "parseServerLine listen: tokens.size() before: " << tokens.size() << std::endl;
		tokens = splitString(newLine, ' ');
		// std::cout << "tokens.size() after: " << tokens.size() << std::endl;
		if (tokens.size() != 2 || tokens[1].back() != ';')
		{
			std::cerr << "Invalid listen directive" << std::endl;
			exit(1);
		}
		std::string portStr = tokens[1];
		portStr.pop_back(); // remove ;
		try
		{
			int port = std::stoi(portStr);
			currentServer.setPort(port);
		}
		catch (...)
		{
			std::cerr << "Invalid port value" << std::endl;
			exit(1);
		}
	}
	else if (line.find("root ") == 0)
	{
		newLine = removeComment(line);
		// std::cout << "parseServerLine root: tokens.size() before: " << tokens.size() << std::endl;
		tokens = splitString(newLine, ' ');
		// std::cout << "tokens.size() after: " << tokens.size() << std::endl;
		if (tokens.size() != 2 || tokens[1].back() != ';')
		{
			std::cerr << "Invalid root directive" << std::endl;
			exit(1);
		}
		std::string path = tokens[1];
		path.pop_back();
		if (path.empty())
		{
			std::cerr << "Empty root path" << std::endl;
			exit(1);
		}
		currentServer.setRoot(path);
	}
	else if (line.find("index ") == 0)
	{
		newLine = removeComment(line);
		// std::cout << "parseServerLine index: tokens.size() before: " << tokens.size() << std::endl;
		tokens = splitString(newLine, ' ');
		// std::cout << "tokens.size() after: " << tokens.size() << std::endl;
		if (tokens.size() != 2 || tokens[1].back() != ';')
		{
			std::cerr << "Invalid index directive" << std::endl;
			exit(1);
		}
		std::string path = tokens[1];
		path.pop_back();
		if (path.empty())
		{
			std::cerr << "Empty index path" << std::endl;
			exit(1);
		}
		currentServer.setIndex(path);
	}
	else if (line.find("server_name ") == 0)
		return ; // not supported yet
	else if (line.find("client_max_body_size ") == 0)
	{
		newLine = removeComment(line);
		// std::cout << "parseServerLine client_max_body_size: tokens.size() before: " << tokens.size() << std::endl;
		tokens = splitString(newLine, ' ');
		// std::cout << "tokens.size() after: " << tokens.size() << std::endl;
		if (tokens.size() != 2 || tokens[1].back() != ';')
		{
			std::cerr << "Invalid client_max_body_size directive" << std::endl;
			exit(1);
		}
		std::string sizeStr = tokens[1];
		sizeStr.pop_back();
		try
		{
			size_t size = std::stoul(sizeStr);
			currentServer.setClientMaxBodySize(size);
		}
		catch(...)
		{
			std::cerr << "Invalid client_max_body_size" << std::endl;
			exit(1);
		}
	}
	else if (line.find("error_page ") == 0)
	{
		newLine = removeComment(line);
		// std::cout << "parseServerLine error_page: tokens.size() before: " << tokens.size() << std::endl;
		tokens = splitString(newLine, ' ');
		// std::cout << "tokens.size() after: " << tokens.size() << std::endl;
		if (tokens.size() != 3 || tokens[2].back() != ';')
		{
			std::cerr << "Invalid error_page directive" << std::endl;
			exit(1);
		}
		int code;
		try
		{
			code = std::stoi(tokens[1]);
			if (code < 400 || code > 599)
			{
				std::cerr << "Invalid error code in error_page" << code << std::endl;
				exit(1);
			}
		}
		catch (...)
		{
			std::cerr << "Invalid error_page" << std::endl;
			exit(1);
		}
		std::string path = tokens[2];
		// std::cout << "error_page: path: " << path << std::endl;
		path.pop_back();
		if (path.empty())
		{
			std::cerr << "Empty path in error_page directive" << std::endl;
			exit(1);
		}
		currentServer.setErrorPage(code, path);
	}
	else
	{
		std::cerr << "Uknown directive in server block: " << line << std::endl;
		exit(1);
	}
}

void ConfigParser::parseLocationLine(const std::string &line, LocationConfig &currentLocation)
{
	std::vector<std::string> tokens;
	std::string newLine;
	if (line.find("root ") == 0)
	{
		newLine = removeComment(line);
		// std::cout << "parseLocationLine root: tokens.size() before: " << tokens.size() << std::endl;
		tokens = splitString(newLine, ' ');
		// std::cout << "tokens.size() after: " << tokens.size() << std::endl;;
		if (tokens.size() != 2 || tokens[1].back() != ';')
		{
			std::cerr << "Invalid root directive" << std::endl;
			exit(1);
		}
		std::string path = tokens[1];
		path.pop_back();
		if (path.empty())
		{
			std::cerr << "Empty root path" << std::endl;
			exit(1);
		}
		currentLocation.setRoot(path);
	}
	else if (line.find("methods ") == 0)
	{
		newLine = removeComment(line);
		// std::cout << "parseLocationLine methods: tokens.size() before: " << tokens.size() << std::endl;
		tokens = splitString(newLine, ' ');
		// std::cout << "tokens.size() after: " << tokens.size() << std::endl;;
		if (tokens.empty())
		{
			std::cerr << "Invalid methods directive: empty tokens" << std::endl;
			exit(1);
		}
		tokens.erase(tokens.begin()); // remove 'methods'
		std::string &lastToken = tokens.back();
		if (!lastToken.empty() && lastToken.back() == ';')
			lastToken.pop_back();
		else
		{
			std::cerr << "Invalid methods directive: missing ';'" << std::endl;
			exit(1);
		}
		currentLocation.setMethods(tokens);
	}
	else if (line.find("index ") == 0)
	{
		newLine = removeComment(line);
		// std::cout << "parseLocationLine index: tokens.size() before: " << tokens.size() << std::endl;
		tokens = splitString(newLine, ' ');
		// std::cout << "tokens.size() after: " << tokens.size() << std::endl;;
		if (tokens.size() != 2 || tokens[1].back() != ';')
		{
			std::cerr << "Invalid index directive" << std::endl;
			exit(1);
		}
		std::string path = tokens[1];
		path.pop_back();
		if (path.empty())
		{
			std::cerr << "Empty index path" << std::endl;
			exit(1);
		}
		currentLocation.setIndex(path);
	}
	else if (line.find("upload ") == 0)
	{
		newLine = removeComment(line);
		// std::cout << "parseLocationLine upload: tokens.size() before: " << tokens.size() << std::endl;
		tokens = splitString(newLine, ' ');
		// std::cout << "tokens.size() after: " << tokens.size() << std::endl;;
		if (tokens.size() != 2 || tokens[1].back() != ';')
		{
			std::cerr << "Invalid upload directive" << std::endl;
			exit(1);
		}
		std::string upload = tokens[1];
		upload.pop_back();
		if (upload.empty())
		{
			std::cerr << "Empty upload" << std::endl;
			exit(1);
		}
		if (upload != "on" && upload != "off")
		{
			std::cerr << "Incorrect value for upload" << std::endl;
			exit(1);
		}
		currentLocation.setUpload(upload == "on");
	}
	else if (line.find("autoindex ") == 0)
	{
		newLine = removeComment(line);
		// std::cout << "parseLocationLine autoindex: tokens.size() before: " << tokens.size() << std::endl;
		tokens = splitString(newLine, ' ');
		// std::cout << "tokens.size() after: " << tokens.size() << std::endl;;
		if (tokens.size() != 2 || tokens[1].back() != ';')
		{
			std::cerr << "Invalid autoindex directive" << std::endl;
			exit(1);
		}
		std::string autoindex = tokens[1];
		autoindex.pop_back();
		if (autoindex.empty())
		{
			std::cerr << "Empty autoindex" << std::endl;
			exit(1);
		}
		if (autoindex != "on" && autoindex != "off")
		{
			std::cerr << "Incorrect value for autoindex" << std::endl;
			exit(1);
		}
		currentLocation.setAutoIndex(autoindex == "on");
	}
	else if (line.find("cgi ") == 0)
	{
		newLine = removeComment(line);
		// std::cout << "parseLocationLine cgi: tokens.size() before: " << tokens.size() << std::endl;
		tokens = splitString(newLine, ' ');
		// std::cout << "tokens.size() after: " << tokens.size() << std::endl;;
		if (tokens.size() != 3 || tokens[2].back() != ';')
		{
			std::cerr << "Invalid cgi directive. Format: cgi <extension> <bin>;" << std::endl;
			exit(1);
		}
		std::string extenstion = tokens[1];
		std::string bin = tokens[2];
		bin.pop_back(); // ';'
		if (extenstion.empty() || bin.empty())
		{
			std::cerr << "cgi extension or binary path is empty" << std::endl;
			exit(1);
		}
		currentLocation.setCgiExtension(extenstion);
		currentLocation.setCgiBin(bin);
	}
	else
	{
		std::cerr << "Uknown directive in location block: " << line << std::endl;
		exit(1);
	}
}

ConfigParser::ConfigParser(const std::string &configFile)
{
	std::ifstream file(configFile);
	if (!file)
	{
		std::cerr << "Couldn't open file" << std::endl;
		exit(1);
	}
	std::string line;
	std::stack<Level> levelStack;
	ServerConfig currentServer;
	LocationConfig currentLocation;
	while (std::getline(file, line))
	{
		line = trim(line); // clear line
		if (line.empty() || line[0] == '#') // clear line
			continue;


		if (line == "server {")
		{
			currentServer = ServerConfig(); // new server
			levelStack.push(SERVER);
			std::cout << "Created new server" << std::endl;
			continue;
		}
		else if (line.find("location ") == 0)
		{
			currentLocation = LocationConfig(); // new Location
			std::string newLine = removeComment(line);
			std::vector<std::string> tokens = splitString(newLine, ' ');
			if (tokens.size() == 1)
			{
				std::cerr << "Invalid location directive" << std::endl;
				exit(1);
			}
			std::string path = tokens[1];
			currentLocation.setPath(path);
			levelStack.push(LOCATION);
			std::cout << "\nCreated new location " << path << std::endl;
			continue;
		}
		else if (line == "}")
		{
			if (levelStack.empty())
			{
				std::cerr << "Unexpected '}'" << std::endl;
				exit(1); 
			}
			Level level = levelStack.top();
			levelStack.pop();
			if (level == LOCATION)
			{
				if (!currentLocation.isValid())
					exit(1);
				try
				{
					LocationConfig copy = currentLocation;
					currentServer.addLocation(copy);
				}
				catch (std::exception &e)
				{
					std::cerr << "Exception during addLocation: " << e.what() << std::endl;
					exit(1);
				}
				catch (...)
				{
					std::cerr << "Unknown exception during addLocation" << std::endl;
					exit(1);
				}
				std::cout << "Added new location" << std::endl;
			}
			else if (level == SERVER)
			{
				if (!currentServer.isValid())
					exit(1);
				webserv.addServer(currentServer);
				currentServer = ServerConfig();
				std::cout << "Added new server" << std::endl;
			}
			continue;
		}
		if (!levelStack.empty())
		{
			Level current = levelStack.top();
			if (current == SERVER)
			{
				parseServerLine(line, currentServer);
			}
			else if (current == LOCATION)
			{
				parseLocationLine(line, currentLocation);
			}
		}
	}
	if (!levelStack.empty())
	{
		std::cerr << "Error: unclosed block in config" << std::endl;
		exit(1);
	}
	file.close();
}
