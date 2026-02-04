/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pauladrettas <pauladrettas@student.42.f    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 17:07:24 by pauladretta       #+#    #+#             */
/*   Updated: 2026/02/04 22:01:01 by pauladretta      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <iostream>
#include <vector>

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
        // pid value
        // pipe fds
        // time
        // ...
        
};

#endif