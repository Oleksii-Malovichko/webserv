_This project has been created as part of the 42 curriculum by pdrettas, tfarkas, omalovic._

# Webserv

## Description
`Webserv` is a custom HTTP/1.1 web server built in C++ as part of the 42 curriculum. The server is designed using an event-driven, non-blocking architecture based on epoll, enabling efficient and seamless communication with multiple clients simultaneously. It supports hosting static websites for multiple users, allows file uploads and deletion, and provides directory listing functionality when autoindex is enabled in the configuration file.

## Goal
The goal of this project is to build a web server from scratch capable of communicating with clients over the Transmission Control Protocol (TCP). Therefore, understanding HTTP, client-server communication, and system programming while handling requests efficiently and reliably.

## Project Structure 



## Instructions
### Build
```bash
make
```

### Execute
```bash
./webserv config/webserv.config     # [configuration file]
```

### Extra
```bash
curl -v -X POST http://localhost:8080/upload -F "file=@test_uploadfile.txt"        # HTTP File Upload
```

## Usage Examples





## Technical Choices





## Resources
- Allowed function manual pages
https://man7.org/linux/man-pages/
https://www.geeksforgeeks.org/cpp/socket-programming-in-cpp/
https://linux.die.net/man/7/epoll
https://pubs.opengroup.org/onlinepubs/009604599/functions/freeaddrinfo.html
https://man.freebsd.org/cgi/man.cgi?kqueue

- CGI
https://en.wikipedia.org/wiki/Common_Gateway_Interface
https://datatracker.ietf.org/doc/html/rfc3875
https://www.ibm.com/docs/en/i/7.6.0?topic=programming-writing-persistent-cgi-programs
https://www.youtube.com/watch?v=cKckh5pD7VI
https://www.youtube.com/watch?v=oRQbFwfasvo

- Socket programming
https://www.youtube.com/watch?v=NvZEZ-mZsuI
https://www.youtube.com/watch?v=gntyAFoZp-E
https://www.youtube.com/watch?v=_iHMMo7SDfQ

- IO multiplexing
https://www.youtube.com/watch?v=oD94jnuOHLM

- Non-blocking IO
https://www.youtube.com/watch?v=wB9tIg209-8

- How Epoll works
https://www.youtube.com/watch?v=WuwUk7Mk80E


#### How AI was used for this project