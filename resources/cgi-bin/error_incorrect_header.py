# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    error_incorrect_header.py                          :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/25 21:17:50 by pdrettas          #+#    #+#              #
#    Updated: 2026/02/25 21:19:25 by pdrettas         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Tests: Incorrect Header
# Does CGI handle no colon, no blank newline below content-type, and invalid content-type correctly?

print("Content-Type text/plain")  # missing colon
print("Hello")