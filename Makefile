NAME =	webserv

SRC = $(wildcard server/src/*.cpp)\
	  $(wildcard server/src/config/*.cpp)

OBJ_DIR = server/obj

SRC_DIR = server/src

#OBJ = $(patsubst %.cpp,$(OBJ_DIR)/%.o, $(SRC))

#OBJ = $(OBJ:server/obj/server/src/%=$(OBJ_DIR)/%)

OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

MKDIR = $(sort $(dir $(OBJ)))

CC = clang++ -g
FLAGS = #-Wall -Wextra -Werror -std=c++98

#colors for beauty
YELLOW =	\033[33;1m
RESET =		\033[0m
RED =		\033[31;1m
GREEN =		\033[32;1m
MAGENTA =	\033[35;1m

ERASE = \33[2K

.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(MKDIR) $(OBJ)
	@$(CC) $(FLAGS) $(OBJ) -o $(NAME)
	@echo "\n$(MAGENTA)$(NAME) $(GREEN)compiled$(RESET)"

$(MKDIR):
	@mkdir -p $@

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.cpp
	@$(CC) $(FLAGS) -c $< -o $@
	@printf "$(ERASE)$(RED)>> $(YELLOW)[$@]$(GREEN)$(RESET)\r"

include $(wildcard $(D_FILES))

clean:
	@rm -rf $(OBJ_DIR)
	@echo "$(YELLOW)obj $(RED)deleted$(RESET)"

fclean: clean
	@rm -rf $(NAME) $(NAME).dSYM
	@echo "$(MAGENTA)$(NAME) $(RED)deleted$(RESET)"

re: fclean all
