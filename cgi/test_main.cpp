/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_main.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 17:10:03 by pauladretta       #+#    #+#             */
/*   Updated: 2026/02/10 21:15:27 by pdrettas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"

// main is only for testing/debugging purposes
// when compiling: 1. make, 2. ./cgi cgi_scripts/test_file.py (or any other test file)
int main()
{
    // ---------------------------------------------------------------------
    // This part is for testing before connecting to server
    // STEP: create testing envp
        char* envp[] = {
        (char*)"REQUEST_METHOD=POST",
        (char*)"CONTENT_LENGTH=13",
        (char*)"CONTENT_TYPE=application/x-www-form-urlencoded",
        nullptr
    };

    // STEP: create testing POST body
    std::string postBody = "name=Paula&age=26";
    std::string getBody = "";
    
    // STEP: create argv
    char *argv[] = {
        (char*)"/usr/bin/python3",
        // (char*)"cgi_scripts/error_crash.py",
        // (char*)"cgi_scripts/error_infinite_loop.py",
        // (char*)"cgi_scripts/test_basic.py",
        // (char*)"cgi_scripts/test_env.py",
        // (char*)"cgi_scripts/test_large_output.py",
        (char*)"cgi_scripts/test_memory_heavy.py",
        nullptr
    };
    
    
    // -------------------------------------------------------------------
    // TODO: everything below will be put into the Server::handleCGI(Client &client) function

    // create instance (input given from server)
    CgiHandler cgi(postBody, envp, argv[1], argv);
    
    // FT: safety check
    if (!cgi.validateExecveArgs(argv, envp))
        return 1;

    // FT: execute cgi
    if (!cgi.execute())
    {
        std::cerr << "CGI execution failed!" << std::endl;
        return 1;
    }

    // FT: translate cgi output as http response


    
    return 0;
}
