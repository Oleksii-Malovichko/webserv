#include "LocationConfig.hpp"

LocationConfig::LocationConfig()
{
	this->path = ""; // if no path -> error!
	this->methods = {"GET", "POST", "DELETE"};
	this->upload = false;
	this->upload_dir = "";
	this->auto_index = false;
	this->index = "index.html";
}

void LocationConfig::setPath(const std::string &p)
{
	this->path = p;
}

void LocationConfig::setRoot(const std::string &r)
{
	this->root = r;
}

void LocationConfig::setMethods(const std::vector<std::string> &m)
{
	this->methods = m;
}

void LocationConfig::setUploadDir(const std::string &dir)
{
	this->upload_dir = dir;
}

void LocationConfig::setUpload(bool on)
{
	this->upload = on;
}

void LocationConfig::setAutoIndex(bool on)
{
	this->auto_index = on;
}

void LocationConfig::setIndex(const std::string &file)
{
	this->index = file;
}

void LocationConfig::addCgi(const std::string &ext, const std::string &bin)
{
	cgi[ext] = bin;
}


void LocationConfig::setRedirect(int code, const std::string &url)
{
	redirect_enable = true;
	redirect_code = code;
	redirect_url = url;
}

bool LocationConfig::hasRedirect() const
{
	return redirect_enable;
}

int LocationConfig::getRedirectCode() const
{
	return redirect_code;
}

const std::string &LocationConfig::getRedirectUrl() const
{
	return redirect_url;
}


const std::string &LocationConfig::getPath() const
{
	return this->path;
}

const std::string &LocationConfig::getRoot() const
{
	return this->root;
}

const std::vector<std::string> &LocationConfig::getMethods() const
{
	return this->methods;
}

bool LocationConfig::getUpload() const
{
	return this->upload;
}

const std::string &LocationConfig::getUploadDir() const
{
	return this->upload_dir;
}

bool LocationConfig::getAutoIndex() const
{
	return this->auto_index;
}

const std::string &LocationConfig::getIndex() const
{
	return this->index;
}

const std::unordered_map<std::string, std::string> &LocationConfig::getCgi() const
{
	return this->cgi;
}

bool LocationConfig::isCgi(const std::string &path) const
{
	for (auto it = cgi.begin(); it != cgi.end(); it++)
	{
		if (path.size() >= it->first.size() && path.compare(path.size() - it->first.size(), it->first.size(), it->first) == 0)
			return 1;
	}
	return 0;
}

bool LocationConfig::isValid() const
{
	if (path.empty())
	{
		std::cerr << "Location path is missing" << std::endl;
		return false;
	}
	if (path[0] != '/')
	{
		std::cerr << "Invalid location path" << std::endl;
		return false;
	}
	// check CGI: keys and values must not be empty
	for (std::unordered_map<std::string, std::string>::const_iterator it = cgi.begin(); it != cgi.end(); ++it)
	{
		if (it->first.empty() || it->second.empty())
		{
			std::cerr << "Invalid CGI configuration: extension or binary is empty" << std::endl;
			return false;
		}
	}
	return true;
}

void LocationConfig::printLocationConfig(void) const
{
	std::cout	<< "Location information:"
				<< "\nPath: " << this->path
				<< "\nRoot: " << this->root
				<< "\nAllowed methodes: ";
				
	for (auto it = this->methods.begin(); 
			it != this->methods.end(); ++it)
	{
		std::cout << *it << " ";
	}

	std::cout	<< std::boolalpha
				<< "\nPossible to upload: " << this->upload
				<< "\nUpload direction: " << this->upload_dir
				<< "\nPossible autoindex: " << this->auto_index
				<< "\nIndex: " << this->index;
				
	for (auto it = this->cgi.begin(); it != this->cgi.end(); ++it)
	{
		std::cout	<< "\nCGI extension: " << it->first 
					<< " CGI binary: " << it->second;
	}

	std::cout << std::endl;
}
