_This project has been created as part of the 42 curriculum by pdrettas, tfarkas, omalovic._

# Webserv

## Description
`Webserv` is a custom HTTP/1.1 web server built in C++ as part of the 42 curriculum. The server is designed using an event-driven, non-blocking architecture based on epoll, enabling efficient and seamless communication with multiple clients simultaneously. It supports hosting static websites for multiple users, allows file uploads and deletion, and provides directory listing functionality when autoindex is enabled in the configuration file.

## Goal
The goal of this project is to build a web server from scratch capable of communicating with clients over the Transmission Control Protocol (TCP). Therefore, understanding HTTP, client-server communication, and system programming while handling requests efficiently and reliably.

## Project Structure 
```text
webserv/
├── CGI/              # xxx
├── Client/           # xxx
├── ConfigParser/     # xxx
├── HttpResponce/     # xxx
├── LocationConfig/   # xxx
├── RAll/             # xxx
├──── epoll/
├──── sockets/
├── Server/           # xxx
├── ServerConfig/     # xxx
├── Uploads/          # xxx
├── WebservConfig/    # xxx
├── client.c          # xxx
├── main.cpp          # xxx
├── Makefile          # xxx
```

## Instructions
### Build
```bash
make
```

### Execute
```bash
./webserv config/webserv.config     # [configuration file]
```

### Usage Examples
```bash
curl -X GET "http://localhost:8080/api/resource"            # GET request (retrieve data)
```

```bash
curl -X POST "http://localhost:8080/api/resource" \
     -H "Content-Type: application/json" \
     -d '{"key1": "value1", "key2": "value2"}'              # POST request with json body
```

```bash
curl -v -X POST "http://localhost:8080/upload" \
     -F "file=@test_uploadfile.txt"                         # POST request with HTTP File Upload
```

```bash
curl -X DELETE "http://localhost:8080/submit-form" \
     -H "Content-Type: application/json" \
     -d '{"confirm": true}'                                 # DELETE request (remove resources)
```

## Webserv Concepts

| Concept / Method | Description | Example Status Codes |
|-----------------|------------|-------------------|
| **GET** | Retrieve data from the server; can include query parameters. | 200 OK, 404 Not Found |
| **POST** | Send structured data in the request body (JSON, form data, file upload). | 201 Created, 400 Bad Request |
| **DELETE** | Remove a resource; may include body or headers for confirmation. | 204 No Content, 403 Forbidden, 404 Not Found |
| **Query Parameters** | Data appended to the URL (`?key=value`); useful for filtering/searching. | N/A |
| **Request Headers** | Key-value metadata sent by the client (`Content-Type`, `Authorization`, etc.). | N/A |
| **Request Body** | Payload sent in POST; includes JSON, URL-encoded form data, or files. | 201 Created, 400 Bad Request |
| **CGI (Common Gateway Interface)** | Executes scripts to generate dynamic content; environment and input passed from Webserv. | 200 OK, 500 Internal Server Error |
| **File Uploads** | POST with `multipart/form-data` to send files; server parses and saves content. | 201 Created, 400 Bad Request |
| **Persistent Connections** | Keep-Alive to handle multiple requests/responses on one TCP connection. | 200 OK |
| **Redirections** | Direct clients to another URL using status codes (301, 302). | 301 Moved Permanently, 302 Found |
| **Error Handling** | Custom or standard error pages for client/server errors. | 400, 403, 404, 500 |
| **Non-blocking I/O** | Handles multiple simultaneous connections efficiently without blocking. | N/A |

## Technical Choices
- C with low-level sockets
- Non-blocking I/O / select()
- Custom HTTP parser
- CGI support
- Manual memory management
- Clear modular architecture

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

- Virtualisation of Memory/CPU & Threading
Book: "Operating Systems: Three Easy Pieces"

#### How AI was used for this project
AI tools were used as a learning and support resource throughout the development of this project. 
They assisted in clarifying core Webserv concepts, explaining complex topics, and serving as a complementary tool for understanding and problem-solving - similar to using documentation, or peer discussion.
AI was not used to generate the implementation of the project. All core logic, architecture decisions, system integration, and code were designed and implemented manually.