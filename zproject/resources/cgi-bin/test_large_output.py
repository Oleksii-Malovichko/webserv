# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    test_large_output.py                               :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/10 21:12:13 by pdrettas          #+#    #+#              #
#    Updated: 2026/02/25 21:06:19 by pdrettas         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Tests: Large Output / Stress Test (outputs 100,000 lines)
# Does the server have correct pipe buffering and output handling?

print("Content-Type: text/plain\n", flush=True)

# print 100,000 lines
for i in range(100_000):
    print(f"Line {i}", flush=False)  # for speed

print("Finished big output", flush=True)
