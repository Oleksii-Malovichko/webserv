#include "ConfigParser.hpp"

std::string trim(const std::string &s)
{
	size_t first = 0;
	size_t last = s.size();

	while (first < s.size() && std::isspace(static_cast<unsigned char>(s[first])))
		first++;
	
	while (last > first && std::isspace(static_cast<unsigned char>(s[last - 1])))
		last--;
	
	return s.substr(first, last - first);
}

std::vector<std::string> splitString(const std::string &str, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;

	for (std::string::size_type i = 0; i < str.length(); i++)
	{
		if (str[i] == delimiter)
		{
			if (!token.empty())
			{
				tokens.push_back(token);
				token.clear();
			}
		}
		else
			token += str[i];
	}
	if (!token.empty())
		tokens.push_back(token);
	return tokens;
}

std::string removeComment(const std::string &str)
{
	std::string newStr;
	for (std::string::size_type i = 0; i < str.length(); i++)
	{
		if (str[i] == '#')
			break;
		newStr += str[i];
	}
	return newStr;
}