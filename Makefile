NAME = webserv

SRC = $(wildcard server/src/*.cpp)\
	  $(wildcard server/src/config/*.cpp)

HEADER = 

# FLAGS = -Wall -Werror -Wextra 
FLAGS = -std=c++98

OBJ = $(SRC:.cpp=.o)

all: $(NAME)
$(NAME): $(OBJ) $(HEADER) 
	@clang++ $(OBJ) $(FLAGS) -o $(NAME) -g
%.o:%.cpp $(HEADER)
	@clang++ $(FLAGS) -c $< -o $(<:.cpp=.o) -g
clean: 
	@rm -f $(OBJ)
fclean: clean
	@rm -f $(NAME)
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
