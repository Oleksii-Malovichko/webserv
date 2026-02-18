#include "DirectoryListing.hpp"

Directorylisting::Directorylisting(void)
{
	this->_opened_directory = nullptr;
}

Directorylisting::Directorylisting(
	const std::string directory_name)
{
	this->_opened_directory = opendir(
		directory_name.c_str());

	if (!this->_opened_directory)
	{
		//check the error page
		std::cerr	<< "The opendir function failed: "
					<< strerror(errno);
		return ;
	}

	this->collectDirectory();
}

Directorylisting::Directorylisting(
	const Directorylisting& other)
{
	this->_opened_directory = other._opened_directory;
	this->_directory_vlist = other._directory_vlist;
}

Directorylisting& Directorylisting::operator=(
	const Directorylisting& other)
{
	if (this != &other)
	{
		this->_opened_directory = other._opened_directory;
		this->_directory_vlist = other._directory_vlist;
	}
	return (*this);
}

Directorylisting::~Directorylisting(void)
{
	closedir(this->_opened_directory);
	this->_opened_directory = nullptr;
	this->_directory_vlist.clear();
}

void Directorylisting::collectDirectory(void)
{
	struct dirent* dir_struct;

	while ((dir_struct = readdir(this->_opened_directory)) != NULL)
	{
		std::string f_name = dir_struct->d_name;

		if (f_name == "." || f_name == "..")
			continue;

		this->_directory_vlist.push_back(f_name);
	}
}

std::string Directorylisting::httpResonseL(
	const std::string directory_name)
{
	std::stringstream ss;

	ss	<< "<html>\n"
		<< "<body>\n"
		<< "<h1>Index of " << directory_name << "</h1>"
		<< "<ul>\n";

	for (auto it = this->_directory_vlist.begin(); 
			it != this->_directory_vlist.end(); ++it)
	{
		ss	<< "<li><a href=\"" << *it << "\">"
			<< *it << "</a><li>\n";
	}

	ss	<< "</ul>\n"
		<< "</body>\n"
		<< "</html>\n";

	return (ss.str());
}