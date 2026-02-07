/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 17:07:24 by pauladretta       #+#    #+#             */
/*   Updated: 2026/02/07 21:53:34 by pdrettas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <iostream>
#include <vector>
#include <unistd.h>

/*
- envp: getting this variable from http parser 
    -> later convert to char** when using execve()
- add pid, pipe to class (in a class bc of possibly multiple requests at the same time, and concurrent CGI executions)
- this class says that this objects represents ONE CGI run
*/
class CgiHandler
{
    private:
        std::vector<char*> envp;
        // time
        // exit code (use later)
        // ...
        pid_t _pid; // value child parent
        int _in_cgi[2]; // server writes input [0], and cgi uses output [1]
        int _out_cgi[2]; // cgi writes (output from script) in input [0], server uses output [1]
        

    public:
        CgiHandler();
        ~CgiHandler();
        bool execute();
        
};

#endif