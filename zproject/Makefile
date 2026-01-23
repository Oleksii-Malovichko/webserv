NAME = webserv

SRCS = main.cpp RAII/epoll/Epoll.cpp \
	LocationConfig/LocationConfig.cpp \
	ServerConfig/ServerConfig.cpp \
	RAII/sockets/ClientSocket.cpp \
	RAII/sockets/ListeningSocket.cpp \
	WebServConfig/WebServConfig.cpp \
	Client/Client.cpp \
	Server/Server.cpp

OBJS = $(SRCS:.cpp=.o)

CXX = c++ -std=c++11
CXXFLAGS = -Wall -Wextra -Werror \
	-I. \
	-IClient \
	-IWebServConfig \
	-IServerConfig \
	-ILocationConfig \
	-IRAII/epoll \
	-IRAII/sockets \
	-IServer

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all 