#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include <string>
# include <vector>
# include <iostream>
# include <unistd.h>
# include <fcntl.h>
# include <map>
# include <set>

# define RED "\033[1;31m"
# define GREEN "\033[1;32m"
# define YELLOW "\033[1;33m"
# define BLUE "\033[1;34m"
# define MAGENTA "\033[1;35m"
# define CYAN "\033[1;36m"
# define DEFAULT "\033[0m"

# define BUFFER_SIZE 1024

class httpMethods
{
	private:
		bool _on_get;
		bool _on_post;
		bool _on_put;
		bool _on_delete;

	public:
		httpMethods(void);
		httpMethods(const std::set<std::string>& methods);
		httpMethods(const httpMethods& other);
		httpMethods& operator=(const httpMethods& other);
		~httpMethods(void);
};

class serverLocation
{
	private:
		std::string _path;
		std::string _root;
		std::string _index;
		//std::set<std::string> _methods;
		httpMethods _methods;
		bool _autoindex;
		bool _upload_on;
		std::string _upload_store;

	public:
		serverLocation(void);
		/*
		serverLocation(const std::string& in_path, 
				const std::string& in_root,
				const std::string &in_index,
				const std::set<std::string>& in_methods);
		serverLocation(const bool in_autoindex,
				const bool in_upload_on,
				const std::string& in_upload_store);
		*/
		serverLocation(const serverLocation& other);
		serverLocation& operator=(const serverLocation& other);
		~serverLocation(void);

};

class serverInfo
{
	private:
		std::vector<serverLocation> _locations;
		int _port;
		std::string _server_name;
		std::string _s_root;
		std::string _s_index;
		size_t _max_body_size;
		std::map<int, std::string> _error_pages;


	public:
		serverInfo(void);
		serverInfo(const serverInfo& other);
		serverInfo& operator=(const serverInfo& other);
		~serverInfo(void);

		void parseServerBlock(
			const std::string& serverblock,
			size_t start_server_pos);
		void addLocation();
};

class configParser
{
	private:
		std::vector<serverInfo> _servers;

	public:
		configParser(void);
		configParser(const configParser& other) = delete;
		configParser& operator=(const configParser& other) = delete;
		~configParser(void);

		int checkInputArguments(int argc, char** argv);
		int readConfigFile(const std::string& path);
		void addServer(const std::string& line);

};

size_t skipWhitespace(
	const std::string& str, size_t start_pos);

size_t skipWordAndWhitespace(
	const std::string& str,
	const std::string& search, size_t start_pos);

const std::string& getInfo(
	const std::string& str,
	size_t start_pos, const char delimiter);

#endif