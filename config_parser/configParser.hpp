#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include <string>
# include <sstream>
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

		// Getters for httpMethods class
		bool getGet(void) const;
		bool getPost(void) const;
		bool getPut(void) const;
		bool getDelete(void) const;
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

		void addLocation(std::string& locationblock);

		// Getters for serverLocation class
		const std::string& getPath(void) const;
		const std::string& getRoot(void) const;
		const std::string& getIndex(void) const;
		const httpMethods& getMethods(void) const;
		bool getAutoindex(void) const;
		bool getUploadOn(void) const;
		const std::string& getUploadStore(void) const;
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

		// Getters for serverInfo class
		int getPort(void) const;
		const std::string& getServerName(void) const;
		const std::string& getRoot(void) const;
		const std::string& getIndex(void) const;
		size_t getMaxBodySize(void) const;
		const std::map<int, std::string>& getErrorPages(void) const;
		const std::vector<serverLocation>& getLocations(void) const;
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
		void printServersInfo(void) const;

};

bool isSearchWord(const std::string& str,
	size_t pos, size_t word_len);

size_t skipWhitespace(
	const std::string& str, size_t start_pos);

size_t skipWordAndWhitespace(
	const std::string& str,
	const std::string& search, size_t start_pos);

std::string getInfo(
	const std::string& str,
	size_t start_pos, const char delimiter);

#endif