# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    test_env.py                                        :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/10 21:03:06 by pdrettas          #+#    #+#              #
#    Updated: 2026/02/10 21:03:09 by pdrettas         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import os

print("Content-Type: text/plain\n")

for key, value in os.environ.items():
    print(f"{key} = {value}")
