#!/usr/bin/python3
import os

print("Content-Type: text/plain")
print()
print("Hello from CGI!")
print("Request method:", os.environ.get("REQUEST_METHOD"))
print("Query string:", os.environ.get("QUERY_STRING"))
