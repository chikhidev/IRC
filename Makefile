NAME=ircserv
CC=c++
CFLAGS=-Wall -Wextra -Werror -std=c++98 -O3

SERVICES_COMMANDS=\
	Services/commands/pass.cpp \
	Services/commands/nick.cpp \
	Services/commands/user.cpp \
	Services/commands/quit.cpp \
	Services/commands/join.cpp \
	Services/commands/part.cpp \
	Services/commands/names.cpp \
	Services/commands/topic.cpp \
	Services/commands/mode.cpp \
	Services/commands/prvmsg.cpp \
	Services/commands/kick.cpp \
	Services/commands/invite.cpp \

SRC=Server/Server.cpp \
	Services/Services.cpp \
	Client/Client.cpp \
	Channel/Channel.cpp \
	main.cpp

SRC += $(SERVICES_COMMANDS)

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