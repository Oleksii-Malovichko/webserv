# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    test_basic.py                                      :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/10 21:03:12 by pdrettas          #+#    #+#              #
#    Updated: 2026/02/10 21:03:15 by pdrettas         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import os

# CGI header
print("Content-Type: text/plain\n")

# Body
print("Hello from CGI!")
print(f"REQUEST_METHOD: {os.getenv('REQUEST_METHOD')}")
print(f"QUERY_STRING: {os.getenv('QUERY_STRING')}")

