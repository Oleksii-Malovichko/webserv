# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    error_crash.py                                     :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/10 21:03:24 by pdrettas          #+#    #+#              #
#    Updated: 2026/02/10 21:14:24 by pdrettas         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

print("Content-Type: text/plain\n")

# simulate an error / crash
raise Exception("Intentional crash for testing CGI error handling")