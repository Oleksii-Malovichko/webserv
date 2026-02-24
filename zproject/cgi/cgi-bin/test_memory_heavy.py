# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    test_memory_heavy.py                               :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/10 21:13:34 by pdrettas          #+#    #+#              #
#    Updated: 2026/02/10 21:14:05 by pdrettas         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

print("Content-Type: text/plain\n", flush=True)
print("Starting memory-heavy loop...", flush=True)

# allocate large list
big_list = ["x" * 1024] * 100_000  # ~100MB

print(f"Allocated list of size: {len(big_list)}", flush=True)

# process list 
count = sum(len(x) for x in big_list)
print(f"Total characters: {count}", flush=True)
print("Done memory-heavy loop", flush=True)
