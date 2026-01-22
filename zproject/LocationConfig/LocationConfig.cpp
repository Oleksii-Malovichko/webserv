#include "LocationConfig.hpp"

LocationConfig::LocationConfig()
{
	this->path = ""; // if no path -> error!
	this->methods = {"GET", "POST", "DELETE"};
	this->upload_dir = "";
	this->cgi_extension = "";
	this->cgi_bin = "";
	this->auto_index = false;
	this->default_file = "index.html";
}