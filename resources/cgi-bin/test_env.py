# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    test_env.py                                        :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/10 21:03:06 by pdrettas          #+#    #+#              #
#    Updated: 2026/02/25 20:53:37 by pdrettas         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Tests: Full Environment Dump (prints all environment variables)
# Are all server CGI env vars correctly set (PATH_INFO, SCRIPT_NAME, etc.)?

import os

print("Content-Type: text/plain\n")

for key, value in os.environ.items():
    print(f"{key} = {value}")
