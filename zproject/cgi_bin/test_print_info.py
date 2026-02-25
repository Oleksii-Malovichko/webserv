# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    print_info.py                                      :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/25 20:23:50 by pdrettas          #+#    #+#              #
#    Updated: 2026/02/25 20:23:50 by pdrettas         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Tests: GET/POST Handling & Env Variables
# Are request method, query string, and body parsed correctly?

#!/usr/bin/python3
import os, sys

method = os.environ.get("REQUEST_METHOD")
query = os.environ.get("QUERY_STRING")
body = sys.stdin.read()

print("Content-type: text/plain")
print()
print("Method: ", method)
print("Query-string: ", query)
print("Body: ", body)