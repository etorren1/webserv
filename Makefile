NAME =	webserv

SRC = $(wildcard server/src/*.cpp)\
	  $(wildcard server/src/config/*.cpp)

HEADER = $(wildcard server/src/*.hpp)\
		 $(wildcard server/src/config/*.hpp)

COUNT_FILES = $(words $(SRC))

OBJ_DIR = server/obj

SRC_DIR = server/src

OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

MKDIR = $(sort $(dir $(OBJ)))

CC = clang++ -g
FLAGS = -Wall -Wextra -Werror -std=c++98

#colors for beauty
RED =		\033[31m
GREEN =		\033[32m
YELLOW =	\033[33m
BLUE =		\033[34m
MAGENTA =	\033[35m
CYAN =		\033[36m
GRAY =		\033[37m
RESET =		\033[0m
ERASE = 	\33[2K

all: $(NAME)

$(NAME): $(MKDIR) $(OBJ) $(HEADER)
	@$(CC) $(FLAGS) $(OBJ) -o $(NAME)
	@printf "$(ERASE)\r"
	@echo "$(RED)>>$(RESET) $(COUNT_FILES) files $(GREEN)compiled$(RESET)"

$(MKDIR):
	@mkdir -p $@

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.cpp $(HEADER)
	@$(CC) $(FLAGS) -c $< -o $@
	@printf "$(ERASE)$(RED)>> $(YELLOW)[$@]$(GREEN)$(RESET)\r"

clean: 
	@rm -rf $(OBJ_DIR)
	@echo "$(RED)>>$(RESET) $(OBJ_DIR) $(RED)deleted$(RESET)"

fclean: clean
	@rm -f $(NAME)
	@echo "$(RED)>>$(RESET) $(NAME) $(RED)deleted$(RESET)"

re: fclean all

run: 
	@./$(NAME)

rerun: re 
	@./$(NAME)

ip: #macOS
	@ipconfig getifaddr en0

iplx: #linux
	@hostname -I

.PHONY: clean fclean re run rerun