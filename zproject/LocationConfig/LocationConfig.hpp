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
	std::vector<std::string> methods; // GET, POST, DELETE; default: GET, POST, DELETE
	std::string upload_dir; // путь для загрузки файлов; default: "" (not supporting)
	std::string cgi_extension; // расширение cgi скрипта; default: ""
	std::string cgi_bin; // путь к интерпритатору; default: ""
	bool auto_index; // включить листинг директорий; default: false
	std::string default_file; // дефолтный файл для директорий; default: "index.html"

	public:
		LocationConfig();
};