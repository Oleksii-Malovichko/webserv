/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 17:10:03 by pauladretta       #+#    #+#             */
/*   Updated: 2026/02/07 22:05:32 by pdrettas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// for debugging purposes: test main

#include "CgiHandler.hpp"

int main()
{
    CgiHandler cgi;
    cgi.execute();
    // std::cout << cgi.getOutput() << std::endl;


    return 0;
}