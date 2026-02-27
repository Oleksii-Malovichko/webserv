# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    test_get.py                                        :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/15 23:08:47 by pdrettas          #+#    #+#              #
#    Updated: 2026/02/25 20:58:56 by pdrettas         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Tests: HTML Output / GET (outputs HTML for GET request)
# Is the HTML response correctly formatted for GET requests?

import sys

# simple GET CGI script that outputs HTML content
print("Content-Type: text/html")
print("")
print("<html>")
print("<head><title>CGI Test - GET</title></head>")
print("<body>")
print("<h1>Hello from Python CGI!</h1>")
print("<p>This is a GET request response.</p>")
print("</body>")
print("</html>")