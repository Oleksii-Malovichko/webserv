#include "configParser.hpp"

// c++ -Wall -Werror -Wextra -std=c++17  configParser.cpp testparser.cpp -o testparser

int main(int argc, char** argv)
{
	configParser parser;

	parser.checkInputArguments(argc, argv);
	
	return (0);
}