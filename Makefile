NAME=ircserv
BONUS_NAME=ircbot
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
	Services/commands/ping.cpp \
	Services/commands/cap.cpp \

SRC=Server/Server.cpp \
	Services/Services.cpp \
	Client/Client.cpp \
	Channel/Channel.cpp \
	shared/glob.cpp \
	main.cpp

BONUS_SRC=Bonus/srcs/irc_bot.cpp \
		  Bonus/srcs/main.cpp

SRC += $(SERVICES_COMMANDS)

OBJ=$(SRC:.cpp=.o)
BONUS_OBJ=$(BONUS_SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

$(BONUS_NAME): $(BONUS_OBJ)
	$(CC) $(CFLAGS) $(BONUS_OBJ) -o $(BONUS_NAME)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

b: all clean

bonus: $(BONUS_NAME)

clean:
	rm -f $(OBJ) $(BONUS_OBJ)

fclean: clean
	rm -f $(NAME) $(BONUS_NAME)

re: fclean all