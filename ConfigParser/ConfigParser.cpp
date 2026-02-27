#include "ConfigParser.hpp"

const WebServConfig &ConfigParser::getWebServ() const
{
	return this->webserv;
}

void ConfigParser::parseServerLine(const std::string &line, ServerConfig &currentServer)
{
	std::vector<std::string> tokens;
	std::stringstream error;

	tokens = splitString(line);
	if (tokens.size() == 1)
	{
		error.clear();
		error << "Invalid " << tokens[0] << " in server directive";
		throw std::runtime_error(error.str());
	}
	if (tokens[0] == "listen")
	{
		if (tokens.size() != 2 || tokens[1].back() != ';')
			throw std::runtime_error("Invalid listen directive");
		std::string listenStr = tokens[1];
		listenStr.pop_back(); // remove ;

		std::string ip;
		std::string portStr;
		size_t colonPos = listenStr.find(":");
		if (colonPos != std::string::npos) // if ':' exists
		{
			ip = listenStr.substr(0, colonPos);
			portStr = listenStr.substr(colonPos + 1); // the part after ':' is port
			currentServer.setIP(ip);
		}
		else
			portStr = listenStr;
		try
		{
			int port = std::stoi(portStr);
			if (port < 0 || port > 65535)
				throw std::runtime_error("Invalid port value");
			currentServer.setPort(port);
		}
		catch (const std::invalid_argument &)
		{
			throw std::runtime_error("Port is not a number");
		}
		catch (const std::out_of_range &)
		{
			throw std::runtime_error("Port value out of range");
		}
	}
	else if (tokens[0] == "root")
	{
		if (tokens.size() != 2 || tokens[1].back() != ';')
			throw std::runtime_error("Invalid root directive");
		std::string path = tokens[1];
		path.pop_back();
		if (path.empty())
			throw std::runtime_error("Empty root path");
		currentServer.setRoot(path);
	}
	else if (tokens[0] == "index")
	{
		if (tokens.size() != 2 || tokens[1].back() != ';')
			throw std::runtime_error("Invalid index directive");
		std::string path = tokens[1];
		path.pop_back();
		if (path.empty())
			throw std::runtime_error("Empty index path");
		currentServer.setIndex(path);
	}
	else if (tokens[0] == "server_name")
	{
		if (tokens.size() != 2 || tokens[1].back() != ';')
			throw std::runtime_error("Invalid server_name directive");
		std::string serverName = tokens[1];
		serverName.pop_back();
		if (serverName.empty())
			throw std::runtime_error("Empty server_name");
		currentServer.setServerName(serverName);
	}
	else if (tokens[0] == "client_max_body_size")
	{
		if (tokens.size() != 2 || tokens[1].back() != ';')
			throw std::runtime_error("Invalid client_max_body_size directive");
		std::string sizeStr = tokens[1];
		sizeStr.pop_back();
		try
		{
			size_t size = std::stoul(sizeStr);
			currentServer.setClientMaxBodySize(size);
		}
		catch(...)
		{
			throw std::runtime_error("Invalid client_max_body_size");
		}
	}
	else if (tokens[0] == "error_page")
	{
		if (tokens.size() != 3 || tokens[2].back() != ';')
			throw std::runtime_error("Invalid error_page directive");
		int code;
		try
		{
			code = std::stoi(tokens[1]);
			if (code < 400 || code > 599)
				throw std::runtime_error("Invalid error code in error_page");
		}
		catch (...)
		{
			throw std::runtime_error("Invalid error_page");
		}
		std::string path = tokens[2];
		path.pop_back();
		if (path.empty())
			throw std::runtime_error("Empty path in error_page directive");
		currentServer.setErrorPage(code, path);
	}
	else
	{
		error.clear();
		error << "Uknown directive in server block: " << line;
		throw std::runtime_error(error.str());
	}
}

// tamas latest version of parseLocation, feb 26
void ConfigParser::parseLocationLine(const std::string &line, LocationConfig &currentLocation)
{
	std::vector<std::string> tokens;
	std::string extenstion = "";
	std::string bin = "";

	tokens = splitString(line);
	if (tokens.size() == 1)
	{
		std::cerr << "Invalid " << tokens[0] << " in location directive" << std::endl;
		exit(1);
	}
	if (tokens[0] == "root")
	{
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
	else if (tokens[0] == "methods")
	{
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
	else if (tokens[0] == "index")
	{
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
	else if (tokens[0] == "upload")
	{
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
	else if (tokens[0] == "autoindex")
	{
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
		else if (tokens[0] == "cgi_pass")
	{
		if (tokens.size() != 2 || tokens[1].back() != ';')
		{
			std::cerr << "Invalid cgi_pass directive. Format: cgi_pass <bin>;" << std::endl;
			exit(1);
		}
		bin = tokens[1];
		bin.pop_back();
		if (bin.empty())
		{
			std::cerr << "Cgi binary path is empty" << std::endl;
			exit(1);
		}
	}
	else if (tokens[0] == "cgi_extension")
	{
		if (tokens.size() != 2 || tokens[1].back() != ';')
		{
			std::cerr	<< "Invalid cgi_extenstion directive. "
						<<	" Format: cgi_extension <extension>;" 
						<< std::endl;
			exit(1);
		}
		extenstion = tokens[1];
		extenstion.pop_back();
		if (extenstion.empty())
		{
			std::cerr << "Cgi extension is empty" << std::endl;
			exit(1);
		}
	}
	else if (tokens[0] == "cgi") // not correct
	{
		if (tokens.size() != 3 || tokens[2].back() != ';')
		{
			std::cerr << "Invalid cgi directive. Format: cgi <extension> <bin>;" << std::endl;
			exit(1);
		}
		extenstion = tokens[1];
		bin = tokens[2];
		bin.pop_back(); // ';'
		if (extenstion.empty() || bin.empty())
		{
			std::cerr << "Cgi extension or binary path is empty" << std::endl;
			exit(1);
		}
		currentLocation.addCgi(extenstion, bin);
	}
	else if (tokens[0] == "redirect") // redirect
	{
		if (tokens.size() != 3 || tokens[2].back() != ';')
		{
			throw std::runtime_error("Invalid redirect directive");
		}
		int code;
		try
		{
			code = std::stoi(tokens[1]);
			if (code < 300 || code > 308)
			{
				std::cerr << "Invalid error code in redirect " << code << std::endl;
				exit(1);
			}
		}
		catch (...)
		{
			std::cerr << "Invalid redirect" << std::endl;
			exit(1);
		}
		std::string url = tokens[2];
		url.pop_back();
		if (url.empty())
		{
			std::cerr << "Empty url in redirect directive" << std::endl;
			exit(1);
		}
		currentLocation.setRedirect(code, url);	
	}
	else
	{
		std::cerr << "Uknown directive in location block: " << line << std::endl;
		exit(1);
	}
}

// alex version of parseLocationLine after merge, feb 26
// void ConfigParser::parseLocationLine(const std::string &line, LocationConfig &currentLocation)
// {
// 	std::vector<std::string> tokens;
// 	std::stringstream error;

// 	tokens = splitString(line);
// 	if (tokens.size() == 1)
// 	{
// 		error.clear();
// 		error << "Invalid " << tokens[0] << " in location directive";
// 		throw std::runtime_error(error.str());
// 	}
// 	if (tokens[0] == "root")
// 	{
// 		if (tokens.size() != 2 || tokens[1].back() != ';')
// 			throw std::runtime_error("Invalid root directive");
// 		std::string path = tokens[1];
// 		path.pop_back();
// 		if (path.empty())
// 			throw std::runtime_error("Empty root path");
// 		currentLocation.setRoot(path);
// 	}
// 	else if (tokens[0] == "methods")
// 	{
// 		if (tokens.empty())
// 			throw std::runtime_error("Invalid methods directive: empty tokens");
// 		tokens.erase(tokens.begin()); // remove 'methods'
// 		std::string &lastToken = tokens.back();
// 		if (!lastToken.empty() && lastToken.back() == ';')
// 			lastToken.pop_back();
// 		else
// 			throw std::runtime_error("Invalid methods directive: missing ';'");
// 		currentLocation.setMethods(tokens);
// 	}
// 	else if (tokens[0] == "index")
// 	{
// 		if (tokens.size() != 2 || tokens[1].back() != ';')
// 			throw std::runtime_error("Invalid index directive");
// 		std::string path = tokens[1];
// 		path.pop_back();
// 		if (path.empty())
// 			throw std::runtime_error("Empty index path");
// 		currentLocation.setIndex(path);
// 	}
// 	else if (tokens[0] == "upload")
// 	{
// 		if (tokens.size() != 2 || tokens[1].back() != ';')
// 			throw std::runtime_error("Invalid upload directive");
// 		std::string upload = tokens[1];
// 		upload.pop_back();
// 		if (upload.empty())
// 			throw std::runtime_error("Empty upload");
// 		if (upload != "on" && upload != "off")
// 			throw std::runtime_error("Incorrect value for upload");
// 		currentLocation.setUpload(upload == "on");
// 	}
// 	else if (tokens[0] == "autoindex")
// 	{
// 		if (tokens.size() != 2 || tokens[1].back() != ';')
// 			throw std::runtime_error("Invalid autoindex directive");
// 		std::string autoindex = tokens[1];
// 		autoindex.pop_back();
// 		if (autoindex.empty())
// 			throw std::runtime_error("Empty autoindex");
// 		if (autoindex != "on" && autoindex != "off")
// 			throw std::runtime_error("Incorrect value for autoindex");
// 		currentLocation.setAutoIndex(autoindex == "on");
// 	}
// 	else if (tokens[0] == "cgi") // not correct
// 	{
// 		if (tokens.size() != 3 || tokens[2].back() != ';')
// 			throw std::runtime_error("Invalid cgi directive. Format: cgi <extension> <bin>;");
// 		std::string extenstion = tokens[1];
// 		std::string bin = tokens[2];
// 		bin.pop_back(); // ';'
// 		if (extenstion.empty() || bin.empty())
// 			throw std::runtime_error("Cgi extension or binary path is empty");
// 		currentLocation.addCgi(extenstion, bin);
// 		// std::cout << "extenstion: " << extenstion << std::endl;
// 		// std::cout << "bin: " << bin << std::endl;
// 	}
// 	else if (tokens[0] == "redirect") // redirect
// 	{
// 		if (tokens.size() != 3 || tokens[2].back() != ';')
// 			throw std::runtime_error("Invalid redirect directive");
// 		int code;
// 		try
// 		{
// 			code = std::stoi(tokens[1]);
// 			if (code < 300 || code > 308)
// 			{
// 				error.clear();
// 				error << "Invalid error code in redirect " << code;
// 				throw std::runtime_error(error.str());
// 			}
// 		}
// 		catch (const std::invalid_argument &)
// 		{
// 			throw std::runtime_error("Redirect code is not a number");
// 		}
// 		catch (const std::out_of_range &)
// 		{
// 			throw std::runtime_error("Redirect code out of range");
// 		}
// 		std::string url = tokens[2];
// 		url.pop_back();
// 		if (url.empty())
// 			throw std::runtime_error("Empty url in redirect directive");
// 		currentLocation.setRedirect(code, url);	
// 	}
// 	else
// 	{
// 		error.clear();
// 		error << "Uknown directive in location block: " << line;
// 		throw std::runtime_error(error.str());
// 	}
// }

ConfigParser::ConfigParser(const std::string &configFile)
{
	std::ifstream file(configFile);
	if (!file)
		throw std::runtime_error("Couldn't open file");

	std::string line;
	std::vector<std::string> tokens;
	std::stack<Level> levelStack;
	ServerConfig currentServer;
	LocationConfig currentLocation;
	while (std::getline(file, line))
	{
		line = trim(line); // clear line
		if (line.empty() || line[0] == '#') // clear line
			continue;

		line = removeComment(line);
		tokens = splitString(line);
		if (line.find("server") == 0 && (levelStack.empty() || levelStack.top() != SERVER))
		{
			currentServer = ServerConfig(); // new server
			bool braceOnLine = false;
			if (tokens.size() == 2 && tokens[1] == "{")
				braceOnLine = true;
			else if (tokens.size() > 1)
				throw std::runtime_error("Invalid server directive: too many tokens");
			if (!braceOnLine)
			{
				std::getline(file, line);
				line = trim(removeComment(line));
				if (line != "{")
					throw std::runtime_error("Expected '{' after server");
			}
			levelStack.push(SERVER);
			// std::cout << "\nCREATED NEW SERVER..." << std::endl;
			continue;
		}
		else if (line.find("location") == 0 && levelStack.top() == SERVER)
		{
			currentLocation = LocationConfig(); // new Location
			bool braceOnLine = false;
			if (tokens.size() == 3 && tokens[2] == "{")
				braceOnLine = true;
			else if (tokens.size() == 1)
				throw std::runtime_error("Invalid location directive");
			else if (tokens.size() > 2)
				throw std::runtime_error("Invalid location directive: too many tokens");
			if (!braceOnLine)
			{
				std::getline(file, line);
				line = trim(removeComment(line));
				if (line != "{")
					throw std::runtime_error("Expected '{' after location");
			}
			std::string path = tokens[1];
			currentLocation.setPath(path);
			levelStack.push(LOCATION);
			// std::cout << "\nCREATED NEW LOCATION " << path << std::endl;
			continue;
		}
		else if (line == "}")
		{
			if (levelStack.empty())
				throw std::runtime_error("Unexpected '}'");
			Level level = levelStack.top();
			levelStack.pop();
			if (level == LOCATION)
			{
				currentLocation.isValid();
				LocationConfig copy = currentLocation;
				currentServer.addLocation(copy);
				// std::cout << "Added new location" << std::endl;
			}
			else if (level == SERVER)
			{
				currentServer.isValid();
				ServerConfig copy = currentServer;
				webserv.addServer(copy);			
				// std::cout << "Added new server" << std::endl;
			}
			continue;
		}
		if (!levelStack.empty())
		{
			Level current = levelStack.top();
			if (current == SERVER)
				parseServerLine(line, currentServer);
			else if (current == LOCATION)
				parseLocationLine(line, currentLocation);
		}
	}
	if (!levelStack.empty())
		throw std::runtime_error("Error: unclosed block in config");
	file.close();
}
