#include "Server.hpp"

bool Server::running = true;

Server::Server(const std::string &configFile)
{
	ConfigParser parser(configFile);
	webserv = parser.getWebServ();
	std::vector<ServerConfig> configs = webserv.getServers();

	// создаем listening sockets и добавляем в epoll
	for (size_t i = 0; i < configs.size(); i++)
	{
		ListeningSocket sock(configs[i].getIP(), configs[i].getPort());
		epoll.addListeningSocket(std::move(sock));
		std::string ip = configs[i].getIP();
		if (ip.empty())
			ip = "0.0.0.0";
		std::cout << "Listening on  ip: " << ip << "; port: " << configs[i].getPort() << std::endl;
	}
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
		// epoll делает всю работу
		epoll.handleEvents(1000); // дефолтное значение таймаута, пока нет клиентов
		
		// Сервер решает, что делать с данными
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
	std::vector<std::string> tokens_path = split(tokens[1], "?");

	if (tokens.size() != 3)
		return ;
	req.method = tokens[0];
	if (tokens_path.size() == 2)
	{
		req.path = tokens_path[0];
		req.query_string = tokens_path[1];
	}
	else
	{
		req.path = tokens[1];
		req.query_string = "";
	}
	req.version = tokens[2];
}

std::string toLower(const std::string &s)
{
	std::string res = s;
	std::transform(res.begin(), res.end(), res.begin(),
					[](unsigned char c){ return std::tolower(c); });
	return res;
}
void parseHttpHeaders(std::string header, HttpRequest &req)
{
	std::vector<std::string> lines = split(header, "\r\n");

	parseRequestLine(lines[0], req); // parse POST /submit-form HTTP/1.1
	for (size_t i = 1; i < lines.size(); i++)
	{
		// тут происходит парсинг всего остального после request line
		if (lines[i].empty())
			continue;
		size_t colonPos = lines[i].find(":");
		if (colonPos != std::string::npos)
		{
			std::string key = trim(lines[i].substr(0, colonPos));
			std::string value = trim(lines[i].substr(colonPos + 1));
			if (toLower(key) == "content-length")
			{
				try
				{
					size_t contentLength = std::stoul(value);
					req.contentLength = contentLength;
				}
				catch(...)
				{
					std::cerr << "Invalid client_max_body_size" << std::endl;
				}
				continue;
			}
			req.headers[toLower(key)] = value;
		}
	}
	req.headersParsed = true;
}

void Server::handleClient(Client &client)
{
	std::string buf = client.getReadBuffer();
	HttpRequest &req = client.getRequest();
	HttpResponce resp;
	std::string headerPart;

	ServerConfig current_server = selectServer(req);
	LocationConfig loc = selectLocation(req.path, current_server);
	if (isAllowedMethod(req, loc) == false)
	{
		//405 Not allowed method
		return ;
	}

	if (isCgiExtensionOK(req, loc) == true)
	{
		this->handleCGI(client);
		// only that case if the CGI response consist the header too
		
		return ;
	}

	size_t pos = buf.find("\r\n\r\n");
	if (pos != std::string::npos)
	{
		req.contentLength = 0;
		headerPart = buf.substr(0, pos);
		parseHttpHeaders(headerPart, req);
		size_t bodyStart = pos + 4; // after '\r\n\r\n'
		if (buf.size() - bodyStart >= req.contentLength)
		{
			req.body = buf.substr(bodyStart, req.contentLength); // only body
			if (req.method == "GET")
			{
				resp.setStatus(200, "OK");
				resp.setBody("<html><body><h1>Welcome!</h1></body></html>");
				resp.setHeader("Content-Type", "text/html");
			}
			else if (req.method == "POST")
			{
				resp.setStatus(200, "OK");
				resp.setBody("<html><body><h1>Form Submitted!</h1></body></html>");
				resp.setHeader("Content-Type", "text/html");
			}
			else if (req.method == "DELETE")
			{
				resp.setStatus(200, "OK");
				resp.setBody("<html><body><h1>Element Removed!</h1></body></html>");
				resp.setHeader("Content-Type", "text/html");
			}
			else // error
			{
				resp.setStatus(404, "Not Found");
				resp.setBody("<html><body><h1>Page Not Found</h1></body></html>");
				resp.setHeader("Content-Type", "text/html");
			}
			std::string responceMessage = resp.serialize(req);
			client.appendToWriteBuffer(responceMessage);
			client.clearReadBuffer();
			client.setState(Client::State::WRITING);
			epoll.updateClientEvents(client);

			// HttpResponce resp;
			// const std::string &back_msg =
			// "HTTP/1.1 200 OK\r\n"
			// "Content-Length: 13\r\n"
			// "Content-Type: text/plain\r\n"
			// "Connection: close\r\n"
			// "\r\n"
			// "Hello, world!";
			// std::cout << "\nWHOLE READBUFFER:"<< std::endl;
			// std::cout << buf << std::endl;
			// client.appendToWriteBuffer(back_msg);
		}
		else
			return ;
	}
	std::cout << "\n\nHttpRequest DEBUG:" << std::endl;
	std::cout << "Method: " << req.method << std::endl;
	std::cout << "Path: " << req.path << std::endl;
	std::cout << "Query string: " << req.query_string << std::endl;
	std::cout << "Version: " << req.version << std::endl;
	for (auto it = req.headers.begin(); it != req.headers.end(); it++)
	{
		std::cout << "Key: " << it->first << std::endl;
		std::cout << "Value: " << it->second << std::endl;
	}
	std::cout << "Content-length: " << req.contentLength << std::endl;
	std::cout << "Body: " << req.body << std::endl;
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
		std::cout	<< GREEN << "The following location block founded:"
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
		return (true);
	}
	
	return (false);
}

std::string Server::handleCGI(Client &client)
{
	std::string cgi_http_response = "";
	char *cgi_path = const_cast<char*>(client.getRequest().path.c_str());

	try
	{
		CgiHandler cgi_obj;
		cgi_obj.setArgsAndCgiPath(cgi_path);
		cgi_obj.setEnvp(client);
		cgi_obj.setNonBlockPipe();
		this->epoll.addCgiPipesToEpoll(cgi_obj, client);
		cgi_obj.runExecve();

		if (PRINT_MSG)
		{
			std::cout	<< GREEN << "The HTTP response:\n"
						<< CYAN << cgi_http_response
						<< DEFAULT << std::endl;
		}
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

	return (cgi_http_response);
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