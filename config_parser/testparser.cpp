#include "configParser.hpp"

// c++ -Wall -Werror -Wextra -std=c++17  configParser.cpp testparser.cpp -o testparser

int main(int argc, char** argv)
{
	configParser parser;

	int check_input = parser.checkInputArguments(
		argc, argv);

	if (check_input < 0)
	{
		return (-1);
	}

	/*
	if (check_input == 1)
	{
		std::string default_path = 
			"../configuration_files/default_config";
		if (parser.readConfigFile(default_path) < 0)
		{
			std::cerr << "Failed to read default config file"
					  << std::endl;
			return (-2);
		}
	}
	else
	{
		if(parser.readConfigFile(argv[1]) < 0)
		{
			return (-2);
		}
	}
	*/


	parser.printServersInfo();
	
	return (0);
}