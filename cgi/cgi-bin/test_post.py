# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    test_post.py                                       :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pdrettas <pdrettas@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/15 23:08:52 by pdrettas          #+#    #+#              #
#    Updated: 2026/02/15 23:53:36 by pdrettas         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

import sys

# POST CGI script that reads from stdin & echoes it back
print("Content-Type: text/html")
print("")
print("<html>")
print("<head><title>CGI Test - POST</title></head>")
print("<body>")
print("<h1>POST Request Received</h1>")

# reads POST data from stdin
post_data = sys.stdin.read()
print(f"<p>Received data: {post_data}</p>")
print(f"<p>Data length: {len(post_data)} bytes</p>")

print("</body>")
print("</html>")