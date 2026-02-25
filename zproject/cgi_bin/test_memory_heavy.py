# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    test_memory_heavy.py                               :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/10 21:13:34 by pdrettas          #+#    #+#              #
#    Updated: 2026/02/25 21:07:35 by pdrettas         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Tests: Memory-Heavy Stress Test (allocates large list (100 mb)
# Does the server handle memory, cleanup, and no crashes correctly?

print("Content-Type: text/plain\n", flush=True)
print("Starting memory-heavy loop...", flush=True)

# allocate large list
big_list = ["x" * 1024] * 100_000  # ~100MB

print(f"Allocated list of size: {len(big_list)}", flush=True)

# process list 
count = sum(len(x) for x in big_list)
print(f"Total characters: {count}", flush=True)
print("Done memory-heavy loop", flush=True)
