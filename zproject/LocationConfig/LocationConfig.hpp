#pragma once

// CGI config
#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>

class LocationConfig
{
	// necessary parameters
	std::string path; // for instance: "/uploads", "/cgi-bin"

	// not necessary parameters
	std::string root;
	std::vector<std::string> methods; // GET, POST, DELETE; default: GET, POST, DELETE
	bool upload;
	std::string upload_dir; // путь для загрузки файлов; default: "" (not supporting)
	bool auto_index; // включить листинг директорий; default: false
	std::string index; // дефолтный файл для директорий; default: "index.html" (index)
	std::unordered_map<std::string, std::string> cgi; // cgi: extension: bin
	
	// redirection
	bool redirect_enable;
	int redirect_code;
	std::string redirect_url;

	// duplicates
	bool hasPath;
	bool hasRoot;
	bool hasIndex;
	bool hasUpload;
	bool hasUploadDir;
	bool hasAutoIndex;
	bool hasRedirectBool;
	bool hasMethods;

	public:
		LocationConfig();
		void setPath(const std::string &p);
		void setRoot(const std::string &r);
		void setMethods(const std::vector<std::string> &m);
		void setUpload(bool on);
		void setUploadDir(const std::string &dir);
		void setAutoIndex(bool on);
		void setIndex(const std::string &index);
		void addCgi(const std::string &ext, const std::string &bin);

		void setRedirect(int code, const std::string &url);
		bool hasRedirect() const;
		int getRedirectCode() const;
		const std::string &getRedirectUrl() const;

		const std::string &getPath() const;
		const std::string &getRoot() const;
		const std::vector<std::string> &getMethods() const;
		bool getUpload() const;
		const std::string &getUploadDir() const;
		bool getAutoIndex() const;
		const std::string &getIndex() const;
		const std::unordered_map<std::string, std::string> &getCgi() const;

		bool isCgi(const std::string &path) const;
		void isValid() const;
		~LocationConfig() = default;
};