#ifndef DIRECTORYLISTING_HPP
# define DIRECTORYLISTING_HPP

# include <sys/types.h>
# include <dirent.h>
# include <vector>
# include <string>
# include <string.h>
# include <iostream>
# include <sstream>

class Directorylisting
{
	private:
		DIR* _opened_directory;
		std::vector<std::string> _directory_vlist;

	public:
		Directorylisting(void);
		Directorylisting(const std::string directory_name);
		Directorylisting(const Directorylisting& other);
		Directorylisting& operator=(const Directorylisting& other);
		~Directorylisting(void);

		void collectDirectory(void);
		std::string httpResonseL(const std::string directory_name);

};

#endif