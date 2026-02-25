# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    error_infinite_loop.py                             :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/10 21:03:18 by pdrettas          #+#    #+#              #
#    Updated: 2026/02/25 20:34:18 by pdrettas         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Tests: Infinite Loop / Timeout
# CGI timeout
# Process killing
# Resource cleanup

import time

print("Content-Type: text/plain\n")
print("Starting infinite loop…")

while True:
    time.sleep(1)