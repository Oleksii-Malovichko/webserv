import os, sys

method = os.environ.get("REQUEST_METHOD")
query = os.environ.get("QUERY_STRING")
body = sys.stdin.read()

print("Content-type: text/plain")
print("Method: ", method)
print("Query-string: ", query)
print("Body: ", body)