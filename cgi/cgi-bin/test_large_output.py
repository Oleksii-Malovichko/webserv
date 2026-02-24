# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    test_large_output.py                               :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/10 21:12:13 by pdrettas          #+#    #+#              #
#    Updated: 2026/02/10 21:14:12 by pdrettas         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

print("Content-Type: text/plain\n", flush=True)

# print 100,000 lines
for i in range(100_000):
    print(f"Line {i}", flush=False)  # for speed

print("Finished big output", flush=True)
