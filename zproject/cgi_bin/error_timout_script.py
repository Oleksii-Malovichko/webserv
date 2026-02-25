# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    timout_script.py                                   :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/25 21:11:57 by pdrettas          #+#    #+#              #
#    Updated: 2026/02/25 21:12:00 by pdrettas         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Tests: Timeout / Slow CGI (sleeps 15s after reading body)
# Does the server enforce max execution time?

#!/usr/bin/python3
import os, sys
import time

body = sys.stdin.read()

print("Content-type: text/plain")
print()
print("Body: ", body)
print("Sleeping for 15 seconds")
time.sleep(15)
print("Never will be printed if CGI "
	"time max < 15 seconds")

