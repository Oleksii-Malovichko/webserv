#include "LocationConfig.hpp"

LocationConfig::LocationConfig()
{
	this->path = ""; // if no path -> error!
	this->methods = {"GET", "POST", "DELETE"};
	this->upload = false;
	this->upload_dir = "";
	this->auto_index = false;
	this->index = "index.html";
	this->redirect_enable = 0;
	this->redirect_code = 0;
	this->redirect_url = "";

	this->hasPath = 0;
	this->hasRoot = 0;
	this->hasIndex = 0;
	this->hasUpload = 0;
	this->hasUploadDir = 0;
	this->hasAutoIndex = 0;
	this->hasRedirectBool = 0;
	this->hasMethods = 0;
}

void LocationConfig::setPath(const std::string &p)
{
	if (hasPath)
		throw std::runtime_error("Duplicate path directive in location");
	this->path = p;
	hasPath = 1;
}

void LocationConfig::setRoot(const std::string &r)
{
	if (hasRoot)
		throw std::runtime_error("Duplicate root directive in location");
	this->root = r;
	hasRoot = 1;
}

void LocationConfig::setMethods(const std::vector<std::string> &m)
{
	if (hasMethods)
		throw std::runtime_error("Duplicate methods directive in location");
	this->methods = m;
	hasMethods = 1;
}

void LocationConfig::setUploadDir(const std::string &dir)
{
	if (hasUploadDir)
		throw std::runtime_error("Duplicate upload_dir directive in location");
	this->upload_dir = dir;
	hasUploadDir = 1;
}

void LocationConfig::setUpload(bool on)
{
	if (hasUpload)
		throw std::runtime_error("Duplicate upload directive in location");
	this->upload = on;
	hasUpload = 1;
}

void LocationConfig::setAutoIndex(bool on)
{
	if (hasAutoIndex)
		throw std::runtime_error("Duplicate autoindex directive in location");
	this->auto_index = on;
	hasAutoIndex = 1;
}

void LocationConfig::setIndex(const std::string &file)
{
	if (hasIndex)
		throw std::runtime_error("Duplicate index directive in location");
	this->index = file;
	hasIndex = 1;
}

void LocationConfig::addCgi(const std::string &ext, const std::string &bin)
{
	cgi[ext] = bin;
}


void LocationConfig::setRedirect(int code, const std::string &url)
{
	if (hasRedirectBool)
		throw std::runtime_error("Duplicate redirect directive in location");
	redirect_enable = true;
	redirect_code = code;
	redirect_url = url;
	hasRedirectBool = 1;
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

void LocationConfig::isValid() const
{
	if (path.empty())
		throw std::runtime_error("Location path is missing");
	if (path[0] != '/')
		throw std::runtime_error("Invalid location path");
	// check CGI: keys and values must not be empty
	for (std::unordered_map<std::string, std::string>::const_iterator it = cgi.begin(); it != cgi.end(); ++it)
	{
		if (it->first.empty() || it->second.empty())
			throw std::runtime_error("Invalid CGI configuration: extension or binary is empty");
	}
}
