NAME1 = webserv
NAME2 = client

SRCS1 = main.cpp RAII/epoll/Epoll.cpp \
	LocationConfig/LocationConfig.cpp \
	ServerConfig/ServerConfig.cpp \
	RAII/sockets/ClientSocket.cpp \
	RAII/sockets/ListeningSocket.cpp \
	WebServConfig/WebServConfig.cpp \
	Client/Client.cpp \
	Server/Server.cpp \
	ConfigParser/ConfigParser.cpp \
	ConfigParser/ConfigParserUtils.cpp \
	HttpResponce/HttpResponce.cpp

SRCS2 = client.c

OBJS1 = $(SRCS1:.cpp=.o)
OBJS2 = $(SRCS2:.c=.o)

CC = cc
CFLAGS = -Wall -Wextra -Werror
CXX = c++ -std=c++11 -fsanitize=address
CXXFLAGS = -Wall -Wextra -Werror \
	-I. \
	-IClient \
	-IWebServConfig \
	-IServerConfig \
	-ILocationConfig \
	-IRAII/epoll \
	-IRAII/sockets \
	-IServer \
	-IConfigParser \
	-IHttpResponce

all: $(NAME1) $(NAME2)

$(NAME1): $(OBJS1)
	$(CXX) $(CXXFLAGS) $(OBJS1) -o $(NAME1)

$(NAME2): $(OBJS2)
	$(CC) $(CFLAGS) $(OBJS2) -o $(NAME2)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS1) $(OBJS2)

fclean: clean
	rm -f $(NAME1) $(NAME2)

re: fclean all 