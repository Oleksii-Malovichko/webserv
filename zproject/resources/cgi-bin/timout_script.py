#!/usr/bin/python3
import os, sys
import time

body = sys.stdin.read()

print("Content-type: text/plain")
print()
print("Body: ", body)
print("Sleeping for 15 seconds")
time.sleep(15)
print("Never will be printed if CGI "
	"time max < 15 seconds")

