#include "Server.hpp"

bool fileExists(const std::string &path)
{
	struct stat buffer;
	return stat(path.c_str(), &buffer) == 0;
}

std::string getFileContent(const std::string &path)
{
	std::ifstream file(path.c_str());
	if (!file.is_open())
		return "";
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

std::string toLower(const std::string &s)
{
	std::string res = s;
	std::transform(res.begin(), res.end(), res.begin(),
					[](unsigned char c){ return std::tolower(c); });
	return res;
}

// GET
bool isMethodAllowed(const std::string method, const LocationConfig *loc)
{
	const std::vector<std::string> methods = loc->getMethods();
	for (size_t i = 0; i < methods.size(); i++)
	{
		if (methods[i] == method)
			return 1;
	}
	return 0;
}

std::string generateDefaultErrorPage(int code, const std::string &reason)
{
	std::stringstream ss;
	ss << "<html>\n"
		<< "<head><title>" << code << " " << reason << "</title></head>\n"
		<< "<body>\n"
		<< "<h1>" << code << " " << reason << "</h1>\n"
		<< "<hr>\n"
		<< "<p>webserv</p>\n"
		<< "</body>\n"
		<< "</html>";

	return ss.str();
}

std::string getReasonPhrase(int code)
{
	switch (code)
	{
		case 200: return "OK";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 400: return "Bad Request";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 413: return "Payload Too Large";
		case 500: return "Internal Server Error";
		case 505: return "HTTP Version Not Supported";
		default: return "Error";
	}
}

bool isPathSafe(const std::string &fullPath, const std::string &root)
{
	char resolvedRoot[PATH_MAX];
	std::string fullRoot = PHYSICAL_ROOT + root;

	if (!realpath(fullRoot.c_str(), resolvedRoot))
		return false;

	std::string rootPath(resolvedRoot);
	if (rootPath.back() != '/')
		rootPath += '/';

	std::string existingPart = fullPath;
	while (!existingPart.empty())
	{
		char resolvedPath[PATH_MAX];
		if (realpath(existingPart.c_str(), resolvedPath))
		{
			std::string path(resolvedPath);
			if (path == rootPath.substr(0, rootPath.size() - 1))
				return true;
			return path.compare(0, rootPath.size(), rootPath) == 0;
		}
		if (existingPart.size() > 1 && existingPart.back() == '/')
			existingPart.erase(existingPart.size() - 1);
		size_t pos = existingPart.find_last_of('/');
		if (pos == std::string::npos)
			break;
		existingPart = existingPart.substr(0, pos);
	}
	return false;
}

bool endsWith(const std::string &str, const std::string &suffix)
{
	if (suffix.size() > str.size())
		return false;
	return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string getMimeType(const std::string& path)
{
	if (endsWith(path, ".html"))
		return "text/html";
	if (endsWith(path, ".css"))
		return "text/css";
	if (endsWith(path, ".js"))
		return "application/javascript";
	if (endsWith(path, ".png"))
		return "image/png";
	if (endsWith(path, ".jpg") || endsWith(path, ".jpeg"))
		return "image/jpeg";
	if (endsWith(path, ".gif"))
		return "image/gif";
	if (endsWith(path, ".txt"))
		return "text/plain";
	return "application/octet-stream";
}

void generateAutoIndex(const std::string &dirPath, const std::string &url, HttpResponce &resp)
{
	DIR *dir = opendir(dirPath.c_str());
	if (!dir)
		return ;

	std::stringstream html;
	html << "<html><body><ul>";
	
	struct dirent *entry;
	html << "<li><a href=\"../\">main page</a></li>";
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;
		if (name == "." || name == "..")
			continue;
		std::string link = url;
		if (link.back() != '/')
			link += '/';
		link += name;
		html << "<li><a href=\"" << link << "\">"
			<< name << "</a></li>";
	}
	html << "</ul></body></html>";

	closedir(dir);

	resp.setStatus(200, "OK");
	resp.setHeader("Content-Type", "text/html");
	resp.setBody(html.str());
}

std::string buildFullPath(const LocationConfig *loc, const ServerConfig *server, const std::string &requestPath)
{
	std::string root = loc->getRoot(); // ./resources/www
	if (root.empty())
		root = server->getRoot();
	std::string locationPath = loc->getPath(); // / or /upload etc.
	std::string relativePath;

	// remove prefix location from the path of request
	if (requestPath.find(locationPath) == 0)
		relativePath = requestPath.substr(locationPath.length());
	else
		relativePath = requestPath;

	if (locationPath == "/")
		relativePath = requestPath;
	
	std::string fullPath = PHYSICAL_ROOT + root;
	if (!fullPath.empty() && fullPath.back() != '/')
		fullPath += "/";

	if (!relativePath.empty() && relativePath[0] == '/')
		relativePath = relativePath.substr(1);
	
	fullPath += relativePath;
	return fullPath;
}

void buildRedirect(HttpResponce &resp, const LocationConfig *loc)
{
	int code = loc->getRedirectCode(); // 301 or 302
	std::string url = loc->getRedirectUrl();

	if (url.find("http://") != 0 && url.find("https://") != 0)
	{
		if (url.empty() || url[0] != '/')
			url = "/" + url;
	}

	std::cout << "redirectCode: " << code << std::endl;
	std::cout << "redirectUrl: " << url << std::endl;
	resp.clear();
	resp.setStatus(code, getReasonPhrase(code));
	resp.setHeader("Location", url);
	resp.setHeader("Content-Type", "text/html");

	std::stringstream body;
	body << "<html>\n"
		<< "<head><title>" << code << " "
		<< getReasonPhrase(code)
		<< "</title></head>\n"
		<< "<body>\n"
		<< "<h1>" << code << " "
		<< getReasonPhrase(code)
		<< "</h1>\n"
		<< "<p>Redirecting to <a href=\""
		<< url << "\">"
		<< url << "</a></p>\n"
		<< "</body>\n"
		<< "</html>";

	resp.setBody(body.str());
}

void serveFile(const std::string &path, HttpResponce &resp)
{
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file)
	{
		resp.setStatus(403, "Forbidden");
		resp.setHeader("Content-Type", "text/html");
		resp.setBody("<h1>403 Forbidden</h1>");
		return ;
	}
	
	std::stringstream buffer;
	buffer << file.rdbuf();

	resp.setStatus(200, "OK");
	resp.setHeader("Content-Type", getMimeType(path));
	resp.setBody(buffer.str());
}

void serveFileOrDirectory(const std::string& path, const HttpRequest &req, HttpResponce& resp, const LocationConfig* location, const ServerConfig* server)
{
	struct stat st;

	if (stat(path.c_str(), &st) == -1)
		return buildError(resp, 404, server);

	if (S_ISDIR(st.st_mode))
	{
		std::string indexPath = path + "/" + location->getIndex();

		if (fileExists(indexPath))
			return serveFile(indexPath, resp);

		if (location->getAutoIndex())
		{
			std::cout << "Autoindex is on" << std::endl;
			return generateAutoIndex(path, req.path, resp);
		}
		else
			std::cout << "Autoindex is off" << std::endl;

		return buildError(resp, 403, server);
	}
	else
	{
		serveFile(path, resp);
	}
}


// if (!isPathSafe(fullPath, location->getRoot()))
//     return buildError(resp, 403, server);


/* 
EDGE cases:
GET /
GET /dir/
GET /dir (без /)
GET /../../etc/passwd
GET /file_not_exist
GET /file.php
GET с отключённым методом
GET на route с redirect
GET на route с autoindex
 */