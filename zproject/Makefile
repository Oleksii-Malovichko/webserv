NAME1 = webserv
NAME2 = client_app

# NAME1 = bin/webserv
# NAME2 = bin/client_app

SRCS1 = main.cpp RAII/epoll/Epoll.cpp \
	LocationConfig/LocationConfig.cpp \
	ServerConfig/ServerConfig.cpp \
	RAII/sockets/ClientSocket.cpp \
	RAII/sockets/ListeningSocket.cpp \
	WebServConfig/WebServConfig.cpp \
	Client/Client.cpp \
	Server/Server.cpp \
	ConfigParser/ConfigParser.cpp \
	ConfigParser/ConfigParserUtils.cpp

SRCS2 = client.c

OBJ_DIR = obj

OBJS1 = $(SRCS1:%.cpp=$(OBJ_DIR)/%.o)
OBJS2 = $(SRCS2:%.c=$(OBJ_DIR)/%.o)

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
	-IConfigParser

all: $(NAME1) $(NAME2)

$(NAME1): $(OBJS1)
#	mkdir -p bin
	$(CXX) $(CXXFLAGS) $(OBJS1) -o $(NAME1)

$(NAME2): $(OBJS2)
	$(CC) $(CFLAGS) $(OBJS2) -o $(NAME2)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME1) $(NAME2)
#   rm -rf bin

re: fclean all 