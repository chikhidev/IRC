NAME=ircserv
CC=c++
CFLAGS=-Wall -Wextra -Werror -std=c++98 -O3

SERVICES_SRC=Services/Services.cpp \
	Services/pass.cpp \
	Services/nick.cpp

SRC=Server/Server.cpp \
	Client/Client.cpp \
	main.cpp

SRC += $(SERVICES_SRC)

OBJ=$(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

b: all clean

clean:
	@rm -f $(OBJ)

fclean: clean
	@rm -f $(NAME)

re: fclean all