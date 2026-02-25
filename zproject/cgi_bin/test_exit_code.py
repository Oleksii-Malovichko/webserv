# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    test_exit_code.py                                  :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/25 21:21:20 by pdrettas          #+#    #+#              #
#    Updated: 2026/02/25 21:22:00 by pdrettas         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Tests: Exit Code
# Does the server treat a non-zero exit as a 500 error correctly?

import sys

print("Content-Type: text/plain\n")
sys.exit(42)