#include "Server.hpp"

bool Server::running = true;

Server::Server(const std::string &configFile)
{
	ConfigParser parser(configFile);
	webserv = parser.getWebServ();
	std::vector<ServerConfig> &configs = webserv.getServers();

	// creating listening sockets and add it to epoll
	for (size_t i = 0; i < configs.size(); i++)
	{
		ListeningSocket sock(configs[i].getIP(), configs[i].getPort());
		epoll.addListeningSocket(std::move(sock), &configs[i]);
		std::string ip = configs[i].getIP();
		if (ip.empty())
			ip = "0.0.0.0";
		std::cout << "Listening on  ip: " << ip << "; port: " << configs[i].getPort() << std::endl;
	}
	std::cout << "\n\n";
}

void Server::sigintHandler(int sig)
{
	(void)sig;
	running = 0;
}

void Server::run()
{
	std::signal(SIGINT, sigintHandler);
	while (running)
	{
		// epoll is doing the whole job for events
		epoll.handleEvents(1000); // default value of timeout, till we don't have the clients
		
		// The Server is deciding what to do with clients
		auto &clients = epoll.getClients();

		for (auto it = clients.begin(); it != clients.end(); it++)
		{
			Client &client = it->second;
			if (client.getState() != Client::State::READING)
				continue;
			if (!client.getReadBuffer().empty())
				handleClient(client);
		}
	}
	shutdownServer();
}

LocationConfig *matchLocation(const std::string &path, std::vector<LocationConfig> &locs)
{
	LocationConfig *best = NULL;
	size_t maxLen = 0;
	size_t i = 0;

	while (i < locs.size())
	{
		const std::string &prefix = locs[i].getPath();
		if (path.find(prefix) == 0 && prefix.length() > maxLen)
		{
			best = &locs[i];
			maxLen = prefix.length();
		}
		i++;
	}
	return best;
}

void buildError(HttpResponce &resp, int statusCode, const ServerConfig *server)
{
	resp.clear();
	std::string reason = getReasonPhrase(statusCode);
	resp.setStatus(statusCode, reason);

	std::string errorPagePath = server->getErrorPage(statusCode);
	std::string body;

	if (!errorPagePath.empty() && fileExists(errorPagePath))
		body = getFileContent(errorPagePath);
	else
		body = generateDefaultErrorPage(statusCode, reason);
	resp.setHeader("Content-Type", "text/html");
	resp.setBody(body);
}

std::vector<std::string> split(const std::string &s, const std::string &delimiter)
{
	std::vector<std::string> result;
	size_t start = 0;
	size_t end = s.find(delimiter, 0);
	while (end != std::string::npos)
	{
		result.push_back(s.substr(start, end - start));
		start = end + delimiter.length();
		end = s.find(delimiter, start);
	}
	result.push_back(s.substr(start));
	return result;
}

void parseRequestLine(std::string requestLine, HttpRequest &req)
{
	std::vector<std::string> tokens = split(requestLine, " ");
	if (tokens.size() != 3)
	{
		req.errorCode = 400;
		return ;
	}
	req.method = tokens[0];
	req.path = tokens[1];
	req.version = tokens[2];
	// check for forbitten elements in header
	if (req.path.empty() || req.path == "." || req.path.find(" ") != std::string::npos || req.path[0] != '/')
	{
		req.errorCode = 400;
		return ;
	}
	for (size_t i = 0; i < req.path.size(); i++)
	{
		unsigned char ch = req.path[i];

		// forbidden control-symbols
		if (ch <= 31 || ch == 127)
		{
			req.errorCode = 400;
			return;
		}
		// only these symbols should be allowed
		if (!(std::isalnum(ch) ||
			ch == '-' || ch == '.' || ch == '_' ||
			ch == '~' || ch == '/' || ch == '%' || ch == '?' || ch == '='))
		{
			std::cout << "[parseRequestLine] bad checking...\n";
			req.errorCode = 400;
			return;
		}
	}
	if (req.method != "GET" && req.method != "POST" && req.method != "DELETE")
	{
		req.errorCode = 501;
		return ;
	}
	if (req.version != "HTTP/1.1" && req.version != "HTTP/1.0")
	{
		req.errorCode = 505;
		return ;
	}
	req.errorCode = 0; // no error
}

void parseHttpHeaders(std::string header, HttpRequest &req)
{
	std::vector<std::string> lines = split(header, "\r\n");
	if (lines.empty() || lines[0].empty())
	{
		req.errorCode = 400;
		return ;
	}
	parseRequestLine(lines[0], req); // parse example: {POST /submit-form HTTP/1.1}
	for (size_t i = 1; i < lines.size(); i++)
	{
		// here is happenning the parsing of the rest AFTER request line (METHOD /path version/1.*)
		if (lines[i].empty())
			continue;
		size_t colonPos = lines[i].find(":");
		if (colonPos != std::string::npos)
		{
			std::string key = trim(lines[i].substr(0, colonPos));
			std::string value = trim(lines[i].substr(colonPos + 1));
			if (req.headers.find(toLower(key)) != req.headers.end()) // repeating of key in request
			{
				req.errorCode = 400;
				return ;
			}
			if (toLower(key) == "content-length")
			{
				if (value[0] == '0' && value != "0")
				{
					req.errorCode = 400;
					return ;
				}
				try
				{
					size_t contentLength = std::stoul(value);
					req.contentLength = contentLength;
				}
				catch(...)
				{
					std::cerr << "Invalid Content-Length" << std::endl;
					req.errorCode = 400;
					return ;
				}
			}
			req.headers[toLower(key)] = value;
		}
	}
	if (req.version == "HTTP/1.1")
	{
		if (req.headers.find("host") == req.headers.end())
		{
			req.errorCode = 400;
			return ;
		}
	}
	req.headersParsed = true;
}

void Server::handleGetRequest(HttpRequest &req, HttpResponce &resp, Client &client)
{
	ServerConfig *server = client.getConfig();
	
	// find location
	const LocationConfig *location = matchLocation(req.path, server->getLocations());
	if (!location)
		return buildError(resp, 404, server);

	// check method
	std::cout << "Location: " << location->getPath() << std::endl;
	if (!isMethodAllowed(req.method, location))
	{
		std::cout << "[handleGetRequest] method not allowed" << std::endl;
		return buildError(resp, 405, server);
	}

	// redirect (if it's needed)
	if (location->hasRedirect())
	{
		std::cerr << CYAN << "The redirection called."<< DEFAULT << std::endl;
		return buildRedirect(resp, location);
	}
	
	// figure out filesystem path
	// std::cout << "req.path: " << req.path << std::endl;
	std::string fullPath = buildFullPath(location, server, req.path); // here is used the macros
	// std::cout << "[handleGetRequest] fullPath: " << fullPath << std::endl;

	// get root path from location (if it's empty, get it from server)
	std::string root = location->getRoot();
	if (root.empty())
		root = server->getRoot();
	if (!isPathSafe(fullPath, root)) // here is used the macros
	{
		std::cerr << RED << "The Path not safe: " << YELLOW << fullPath << std::endl;
		return buildError(resp, 403, server);
	}

	// parse: /cgi/python/showenv.py/data/comment?userinfo=hello if token has '?', than it's a query, not the part of path 
	// CGI part
	if (location->isCgi(fullPath))
	{
		std::cerr	<< CYAN << "The CGI called with path: "
					<< YELLOW << fullPath << DEFAULT << std::endl;
		
		req.path = fullPath;
		return handleCGI(req, resp, client);
	}
	
	std::cout << "req.path: " << req.path << std::endl;
	// proccessing file/directory
	serveFileOrDirectory(fullPath, req, resp, location, server);
}

void Server::handlePostRequest(HttpRequest &req, HttpResponce &resp, Client &client)
{
	ServerConfig *server = client.getConfig();

	// find location
	const LocationConfig *location = matchLocation(req.path, server->getLocations());
	if (!location)
		return buildError(resp, 404, server);
	
	// check method
	if (!isMethodAllowed(req.method, location))
		return buildError(resp, 405, server);
	
	// check the correctness of Content-Length and Transfer-Encoding (they're already toLower)
	if (req.headers.find("content-length") == req.headers.end() && req.headers.find("transfer-encoding") == req.headers.end())
		return buildError(resp, 411, server);
	if (req.contentLength != 0 && req.headers.find("transfer-encoding") != req.headers.end())
		return buildError(resp, 400, server);
	if (req.contentLength != 0)
	{
		if (req.body.size() != req.contentLength)
			return buildError(resp, 400, server);	
	}

	// Handle the case with Transfer-Encoding: chunked (not sure that we should do that)
	if (req.headers.find("Transfer-Encoding") != req.headers.end() &&
        req.headers["Transfer-Encoding"] == "chunked")
	{
		;
	}
	
	// figure out filesystem path
	std::string fullPath = buildFullPath(location, server, req.path);
	// std::cout << "[handlePostRequest] fullPath: " << fullPath << std::endl;

	std::string root = location->getRoot();
	if (root.empty())
		root = server->getRoot();
	if (!isPathSafe(fullPath, root))
		return buildError(resp, 404, server);

	if (location->isCgi(fullPath))
		return handleCGI(req, resp, client);

	// *******
	if (!location->getUpload())
		return buildError(resp, 403, server);

	std::string contentType = req.headers["content-type"]; // P: added here instead of right before upload ft
	// std::cout << "\033[32mcontent-type = [" << contentType << "]\033[0m" << std::endl; // TODO: delete later

	// check if file/path exist (here is such error, cause the responce can be POST /check HTTP/1.1\r\nHost:)
	if (!(contentType.find("multipart/form-data") != std::string::npos) && !fileExists(fullPath)) // P: add condition for uploads bc file may not exist YET but will be created later in upload ft
		return buildError(resp, 403, server); // file not exist

	// *********** alex beginning
	// // generate the name of file
	// std::string filename = "upload_" + std::to_string(time(NULL));
	// if (fullPath.back() != '/')
	// 	fullPath += "/";
	// fullPath += filename;

	// // here should be code of Paula with parsing of boundary
	// std::ofstream ofs(fullPath.c_str(), std::ios::binary);
	// if (!ofs)
	// {
	// 	std::cerr << "Error with opening file: " << fullPath << std::endl;
	// 	return buildError(resp, 500, server);
	// }
	// // write the body to the file (must be realised the multipart)
	// ofs << req.body;
	// ofs.close();
	// *********** alex end

	// *********** HTTP FILE UPLOAD HANDLER (beginning) Paula*****************************************************

	if (contentType.find("multipart/form-data") != std::string::npos)
	{
		int status = handleHttpFileUpload(contentType, req.body, resp);
		if (status == -1)
			return buildError(resp, 400, server); // new
	}
// *************** HTTP FILE UPLOAD HANDLER (end) Paula*****************************************************


	// build the responce
	// resp.setStatus(201, "Created");
	// resp.setHeader("Content-Type", "text/html");
	// std::stringstream body;
	// body << "<html><body>"
	// 	<< "<h1>File uploaded succesfully!</h1>"
	// 	<< "</body></html>";
	// resp.setBody(body.str());
}

void Server::handleDeleteRequest(HttpRequest &req, HttpResponce &resp, Client &client)
{
	ServerConfig *server = client.getConfig();

	// find location
	const LocationConfig *location = matchLocation(req.path, server->getLocations());
	if (!location)
		return buildError(resp, 404, server);
	
	if (!isMethodAllowed(req.method, location))
		return buildError(resp, 405, server);
	
	std::string fullPath = buildFullPath(location, server, req.path);
	
	std::string root = location->getRoot();
	if (root.empty())
		root = server->getRoot();
	if (!isPathSafe(fullPath, root))
		return buildError(resp, 403, server);
	
	if (!fileExists(fullPath))
		return buildError(resp, 404, server);
	
	struct stat st;
	std::cout << "Fullpath: " << fullPath << std::endl;
	if (stat(fullPath.c_str(), &st) == -1)
		return buildError(resp, 500, server);
	
	int status = 0;
	if (S_ISDIR(st.st_mode))
		status = rmdir(fullPath.c_str()); // remove only empty directory
	else
		status = remove(fullPath.c_str());
	
	if (status != 0)
	{
		std::cerr << "Failed to delete " << fullPath << ": " << strerror(errno) << std::endl;
		return buildError(resp, 500, server);
	}

	resp.setStatus(204, "No Content");
	resp.setBody("");
}

bool isErrorsHeaders(HttpResponce &resp, HttpRequest &req, Client &client)
{
	bool err = 0;

	if (req.errorCode != 0)
	{
		err = 1;
		buildError(resp, req.errorCode, client.getConfig());
	}
	else if (req.contentLength > 2147483647)
	{
		err = 1;
		buildError(resp, 400, client.getConfig());
	}
	else if (req.contentLength > client.getConfig()->getClientMaxBodySize())
	{
		err = 1;
		buildError(resp, 413, client.getConfig());
	}
	if (err)
	{
		std::string responceMessage = resp.serialize(req);
		client.appendToWriteBuffer(responceMessage);
		client.clearReadBuffer();
		client.setState(Client::State::WRITING);
	}
	return err;
}

void Server::handleClient(Client &client)
{
	std::string buf = client.getReadBuffer();
	HttpRequest &req = client.getRequest();
	HttpResponce resp;
	std::string headerPart;

	// ServerConfig current_server = selectServer(req);
	// current_server.printServerConfig();
	// LocationConfig loc = selectLocation(req.path, current_server);
	// loc.printLocationConfig();
	// client.printHttpRequest();
	// if (isAllowedMethod(req, loc) == false)
	// {
	// 	//405 Not allowed method
	// 	std::cerr << RED << "405 Not allowed method"
	// 				<< DEFAULT << std::endl;
	// 	return ;
	// }

	// if (isDirectoryListing(req, loc) == true)
	// {
	// 	Directorylisting dir_list(req.path);
	// 	client._http_response = dir_list.httpResponseL(req.path);
	// }

	// if (isCgiExtensionOK(req, loc) == true)
	// {
	// 	client._http_response = this->handleCGI(client);
	// 	// only that case if the CGI response consist the header too
	// 	std::cerr << BLUE << "CGI finished" 
	// 				<< DEFAULT << std::endl;
	// 	return ;
	// }

	size_t pos = buf.find("\r\n\r\n");
	if (pos != std::string::npos)
	{
		req.contentLength = 0;
		headerPart = buf.substr(0, pos);
		parseHttpHeaders(headerPart, req);
		if (isErrorsHeaders(resp, req, client)) // find errors after parsing headers
			return epoll.updateClientEvents(client);

		size_t bodyStart = pos + 4; // after '\r\n\r\n'
		if (buf.size() - bodyStart >= req.contentLength && req.headersParsed)
		{
			// std::cout << "\nWHOLE READBUFFER:"<< std::endl;
			// std::cout << buf << std::endl;
			// std::cout << "\nClient connected: " << std::endl;
			req.body = buf.substr(bodyStart, req.contentLength); // only body

			if (req.method == "GET" && !req.errorCode)
			{
				handleGetRequest(req, resp, client);
			}
			else if (req.method == "POST" && !req.errorCode)
			{
				handlePostRequest(req, resp, client);
			}
			else if (req.method == "DELETE" && !req.errorCode)
			{
				handleDeleteRequest(req, resp, client);
				// resp.setStatus(200, "OK");
				// resp.setBody("<html><body><h1>Element Removed!</h1></body></html>");
				// resp.setHeader("Content-Type", "text/html");
			}
			else // error
			{
				resp.setStatus(405, "Method Not Allowed");
				resp.setBody("<html><body><h1>Method Not Allowed</h1></body></html>");
				resp.setHeader("Content-Type", "text/html");
			}
			std::string responceMessage = resp.serialize(req);
			client.appendToWriteBuffer(responceMessage);
			client.clearReadBuffer();
			client.setState(Client::State::WRITING);
			epoll.updateClientEvents(client);
		}
		else
		{
			std::cerr << RED << "HandleClient returned in else"
					<< DEFAULT << std::endl;
			return ;
		}
	}
	client.printHttpRequest();
}

void Server::handleParseRequest(Client &client)
{
	(void)client;
}

void Server::shutdownServer()
{
	std::cout << "Shutting down server..." << std::endl;

	auto &clients = epoll.getClients();
	for (auto &pair : clients)
		pair.second.close();

	epoll.clearEventMap();
}

ServerConfig Server::selectServer(const HttpRequest& req)
{
	std::vector<ServerConfig> servers = webserv.getServers();
	auto it_header = req.headers.find("Host");
	if (it_header == req.headers.end())
	{
		return (servers[0]);
	}

	for (auto it = servers.begin(); it != servers.end(); ++it)
	{
		if (std::to_string(it->getPort()) == it_header->second)
		{
			return (*it);
		}
	}
	
	return (servers[0]);
}

LocationConfig Server::selectLocation(
			const std::string& request_path, 
			const ServerConfig& current_server)
{
	std::vector<LocationConfig> searchlocations = 
		current_server.getLocations();
	LocationConfig found_location;
	size_t found_length = 0;

	for (auto it = searchlocations.begin();
		it != searchlocations.end(); ++it)
	{
		std::string loc_path = it->getPath();
		size_t loc_len  = loc_path.length();
		if (request_path.compare(0, 
			loc_len, loc_path) == 0 && loc_len > found_length) 
		{
			found_location = *it;
			found_length = loc_len;
		}
	}

	if (PRINT_MSG)
	{
		std::cout	<< BLUE << "The request path: " << request_path << "\n"
					<< GREEN << "The following location block founded:"
					<< found_location.getPath()
					<< DEFAULT << std::endl;
	}

	return (found_location);
}

bool Server::isAllowedMethod(const HttpRequest& req, 
			const LocationConfig& loc)
{
	std::vector<std::string> loc_methods = loc.getMethods();

	auto it = std::find(loc_methods.begin(),
		loc_methods.end(), req.method);
	
	if (it != loc_methods.end())
	{
		return (true);
	}

	std::cerr << CYAN << "The request Method: " << req.method
			<< "\nAllowed methods: "
			<< DEFAULT << std::endl;
	for (auto it = loc_methods.begin(); it != loc_methods.end(); ++it)
	{
		std::cerr << YELLOW << *it << "\n";
	}
	return (false);
}

bool Server::isDirectoryListing(const HttpRequest& req, 
			const LocationConfig& loc)
{
	if (req.path.back() == '/')
	{
		if (loc.getAutoIndex() == true)
		{
			return (true);
		}
		else
		{
			throw DirectoryListingOffError();
		}
	}

	return (false);
}

bool Server::isCgiExtensionOK(const HttpRequest& req, 
			const LocationConfig& loc)
{
	std::vector<std::string> split_path = split(req.path, ".");
	if (split_path.size() < 2)
	{
		return (false);
	}

	std::string extract_req_extension = split_path[1];
	extract_req_extension.insert(0, ".");

	if(PRINT_MSG)
	{
		std::cout	<< "The request extension: "
					<< extract_req_extension << std::endl;
	}

	const std::unordered_map<std::string, std::string> cgis = loc.getCgi();
	if (cgis.size() == 0)
	{
		return (false);
	}

	auto it = cgis.find(extract_req_extension);
	if (it != cgis.end())
	{
		return (true);void handlePostRequest(HttpRequest &req, HttpResponce &resp, Client &client);
	}
	
	return (false);
}

// std::string Server::handleCGI(Client &client)
// {
// 	std::string cgi_http_response = "";
// 	char *cgi_path = const_cast<char*>(client.getRequest().path.c_str());

// 	try
// 	{
// 		CgiHandler cgi_obj;
// 		cgi_obj.setArgsAndCgiPath(cgi_path);
// 		cgi_obj.setEnvp(client);
// 		cgi_obj.setNonBlockPipe();
// 		this->epoll.addCgiPipesToEpoll(cgi_obj, client);
// 		cgi_obj.execute();

// 		if (PRINT_MSG)
// 		{
// 			std::cout	<< GREEN << "The HTTP response:\n"
// 						<< CYAN << cgi_http_response
// 						<< DEFAULT << std::endl;
// 		}
// 	}
// 	catch(const std::exception& e)
// 	{
// 		std::cerr	<< RED 
// 					<< "The following error occured " 
// 					<< e.what() << '\n';
// 	}
// 	catch(...)
// 	{
// 		std::cerr << YELLOW << "Some error"; 
// 	}

// 	return (cgi_http_response);
// }

void Server::handleCGI(HttpRequest &req, HttpResponce &resp, Client &client)
{
	(void)req;
	char *cgi_path = const_cast<char*>(client.getRequest().path.c_str());
	(void)client;
	resp.setStatus(200, "OK");
	resp.setHeader("Content-Type", "text/plain");
	// resp.setBody("CGI stub responce\n"); // need to discuss the CGI response will contains the header or not
	// std::string Interpreter = client.getRequest().path;
	std::string Interpreter = "/usr/bin/python3";

	std::cerr << CYAN << "In handleCGI path: " << cgi_path 
				<< "\n Interpreter: " << Interpreter << DEFAULT << std::endl;

	try
	{
		CgiHandler cgi_obj;
		cgi_obj.setInterpreterPath(client.getRequest().path);
		cgi_obj.setArgsAndCgiPath(cgi_path);
		cgi_obj.setEnvp(client);
		cgi_obj.setNonBlockPipe();
		this->epoll.addCgiPipesToEpoll(cgi_obj, client);
		cgi_obj.execute();
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

}

void ServerConfig::printServerConfig(void) const
{
	std::cout	<< "Server information:\n"
				<< "Ip address: " << this->ip
				<<  "\nListen port: " << this->port
				<< "\nRoot: " << this->root
				<< "\nIndex: " << this->index
				<< "\nClient max body size: " << this->client_max_body_size;

	for (auto it = error_pages.begin(); it != error_pages.end(); ++it)
	{
		std::cout << "\nError page code: " << it->first
					<< " Error page: " << it->second;
	}
	std::cout << std::endl;
}


// void Server::handleClient(Client &client)
// {
// 	std::string buf = client.getReadBuffer();
// 	HttpRequest &req = client.getRequest();
// 	std::string headerPart;
// 	size_t pos;
// 	if (!req.headersParsed)
// 	{
// 		pos = buf.find("\r\n\r\n"); // find the end of header
// 		if (pos != std::string::npos)
// 		{
// 			std::cout << "Gotcha the http-header!" << std::endl;
// 			headerPart = buf.substr(0, pos);
			
// 			std::cout << "HEADER PART:\n" << headerPart << "\n\n";
// 			parseHttpHeaders(headerPart, req);

// 			// buf = buf.substr(pos + 4);
// 			// const std::string &back_msg =
// 			// "HTTP/1.1 200 OK\r\n"
// 			// "Content-Length: 13\r\n"
// 			// "Content-Type: text/plain\r\n"
// 			// "Connection: close\r\n"
// 			// "\r\n"
// 			// "Hello, world!";

// 			// client.appendToWriteBuffer(back_msg);
// 			// client.clearReadBuffer();
// 			// client.setState(Client::State::WRITING);
// 			// epoll.updateClientEvents(client);
// 		}
// 		else
// 			return ;
// 	}
// 	if (req.headersParsed && req.contentLength > 0)
// 	{
// 		if (buf.size() - headerPart.size() >= req.contentLength)
// 		{
// 			req.body = buf.substr(0, req.contentLength);
// 			const std::string &back_msg =
// 			"HTTP/1.1 200 OK\r\n"
// 			"Content-Length: 13\r\n"
// 			"Content-Type: text/plain\r\n"
// 			"Connection: close\r\n"
// 			"\r\n"
// 			"Hello, world!";

// 			client.appendToWriteBuffer(back_msg);
// 			client.clearReadBuffer();
// 			client.setState(Client::State::WRITING);
// 			epoll.updateClientEvents(client);
// 		}
// 		else
// 			return ;
// 	}
// }




// void Server::handleClientEcho(Client &client)
// {
// 	const std::string &data = client.getReadBuffer();
// 	if (data == "12345" || data == "12345\n")
// 	{
// 		std::cout << "Client stopped server" << std::endl;
// 		running = false;
// 		return ;
// 	}

// 	// const std::string &back_msg = "Server: " + data;
// 	const std::string &back_msg =
//     "HTTP/1.1 200 OK\r\n"
//     "Content-Length: 13\r\n"
//     "Content-Type: text/plain\r\n"
// 	"Connection: close\r\n"
//     "\r\n"
//     "Hello, world!";

// 	client.appendToWriteBuffer(back_msg);
// 	client.clearReadBuffer();
// 	client.setState(Client::State::WRITING);
// 	epoll.updateClientEvents(client);
// }