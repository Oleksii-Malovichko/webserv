/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_main.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 17:10:03 by pauladretta       #+#    #+#             */
/*   Updated: 2026/02/09 09:52:02 by pdrettas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"

// main is only for testing/debugging purposes
// when compiling: 1. make, 2. ./cgi cgi_scripts/test_file.py (or any other test file)
int main(char **argv, int argc)
{
    // STEP: create testing envp
        char* envp[] = {
        (char*)"REQUEST_METHOD=POST",
        (char*)"CONTENT_LENGTH=13",
        (char*)"CONTENT_TYPE=application/x-www-form-urlencoded",
        nullptr
    };

    // STEP: create testing POST body
    std::string postBody = "name=Paula&age=26";
    
    // STEP: create CGI handler
    char* argv[] = { (char*)"testscript.py", nullptr };
    CgiHandler cgi(postBody, envp, "cgi_scripts/testscript.py", argv); // call this one in server ft

    // STEP: execute cgi
    if (!cgi.execute())
    {
        std::cerr << "CGI execution failed!" << std::endl;
        return 1;
    }

    // STEP: see output of cgi
    // std::cout << cgi.getOutput() << std::endl;

    return 0;
}
