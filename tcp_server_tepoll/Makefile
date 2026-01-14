# NAME1 = libftprintf.a
NAME2 = client
NAME3 = corrct_server

CC = cc 
C_FLAGS = -Wall -Wextra -Werror

# SRCS1 = ft_printf/funcs.c ft_printf/funcs2.c ft_printf/ft_printf.c ft_printf/process_string.c ft_printf/process_string2.c
SRCS2 = client.c
SRCS3 = server.c

# OBJS1 = $(SRCS1:.c=.o)
OBJS2 = $(SRCS2:.c=.o)
OBJS3 = $(SRCS3:.c=.o)


all: $(NAME2) $(NAME3)

# $(NAME1): $(OBJS1)
# 	ar rcs $@ $^

$(NAME2): $(OBJS2)
	$(CC) $(C_FLAGS) -o $(NAME2) $(OBJS2)

$(NAME3): $(OBJS3)
	$(CC) $(C_FLAGS) -o $(NAME3) $(OBJS3)

%.o: %.c
	$(CC) $(C_FLAGS) -c $< -o $@

clean:
	rm -f $(OBJS2) $(OBJS3) 
	rm -f evil_client

fclean: clean
	rm -f $(NAME2) $(NAME3)

re: fclean all

.PHONY: all clean fclean re 