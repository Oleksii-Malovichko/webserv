#pragma once

// CGI config
#include <string>
#include <iostream>
#include <vector>


class LocationConfig
{
	// Обязательные параметры
	std::string path; // for instance: "/uploads", "/cgi-bin"

	// Необязательные параметры
	std::string root;
	std::vector<std::string> methods; // GET, POST, DELETE; default: GET, POST, DELETE
	bool upload;
	std::string upload_dir; // путь для загрузки файлов; default: "" (not supporting)
	bool auto_index; // включить листинг директорий; default: false
	std::string index; // дефолтный файл для директорий; default: "index.html" (index)
	std::string cgi_extension; // расширение cgi скрипта; default: ""
	std::string cgi_bin; // путь к интерпритатору; default: ""

	public:
		LocationConfig();
		void setPath(const std::string &p);
		void setRoot(const std::string &r);
		void setMethods(const std::vector<std::string> &m);
		void setUpload(bool on);
		void setUploadDir(const std::string &dir);
		void setAutoIndex(bool on);
		// void setDefaultFile(const std::string &file);
		void setIndex(const std::string &index);
		void setCgiExtension(const std::string &ext);
		void setCgiBin(const std::string &bin);

		const std::string &getPath() const;
		const std::string &getRoot() const;
		const std::vector<std::string> &getMethods() const;
		bool getUpload() const;
		const std::string &getUploadDir() const;
		bool getAutoIndex() const;
		const std::string &getIndex() const;
		const std::string &getCgiExtension() const;
		const std::string &getCgiBin() const;

		bool isValid() const;
		~LocationConfig() = default;
};