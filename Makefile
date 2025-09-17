NAME=ircserv
CC=c++
CFLAGS=-Wall -Wextra -Werror -std=c++98 -O3

SRC=Server/Server.cpp \
	Client/Client.cpp \
	main.cpp

OBJ=$(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f $(OBJ)

fclean: clean
	@rm -f $(NAME)

re: fclean all