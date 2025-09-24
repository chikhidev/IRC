NAME	=	ircbot

CXX			=	c++
CXXFLAGS	=	-Wall -Wextra -Werror -std=c++98
RM			=	rm -f

INC_DIR		=	includes

SRC_FILES	:=	./srcs/main.cpp \
				./srcs/irc_bot.cpp
INC_FILES	:=	./includes/irc_bot.hpp

OBJS = $(SRC_FILES:.cpp=.o)

INC = -I$(INC_DIR)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(INC) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
