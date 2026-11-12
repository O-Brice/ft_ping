# Nom de l'exécutable
NAME        = ft_ping

# Compilateur et flags
CC          = gcc
CFLAGS      = -Wall -Wextra -Werror -g3 -I includes
LDFLAGS     = -lm

# Dossiers
SRC_DIR     = src
OBJ_DIR     = obj
INC_DIR     = includes

# Fichiers sources
SRC_FILES   = main.c \
              parser.c \
              net.c \
              send_packet.c \
              receive_packet.c \
              utils.c \
              ping_loop.c

SRC         = $(addprefix $(SRC_DIR)/, $(SRC_FILES))
OBJ         = $(addprefix $(OBJ_DIR)/, $(SRC_FILES:.c=.o))

all: $(NAME)

$(NAME): $(OBJ)
	@echo "Création de l'exécutable $(NAME)..."
	@$(CC) $(CFLAGS) $(OBJ) -o $(NAME) $(LDFLAGS)
	@echo "$(MAGENTA)$(BOLD)"
	@echo "                            _,-}}-._"
	@echo "                           /\\   }  /\\"
	@echo "                          _|(O\\_ _/O)                  ___|_"
	@echo "                        _|/  (__''__)                 |#####|"
	@echo "                      _|\\/    WVVVVW                  |#####|"
	@echo "                     \\ _\\     \\MMMM/_             .-----.###|"
	@echo "                   _|\\_\\     _ '---; \\_           |#####|###|"
	@echo "              /\\   \\ _\\/      \\_   /   \\          |#####|###|"
	@echo "             / (    _\\/     \\   \\  |'VVV     P    |#####|###| "
	@echo "            (  '-,._\\_.(      'VVV /      (_/|\\_) |#####|###|"
	@echo "             \\         /   _) /   _)     (_/_ _\\_)|#####|###|"
	@echo "              '....--''\\__vvv)\\__vvv)_____|_| |_|_|#####|###|_____"
	@echo "$(RESET)"
	@echo "Compilation terminée avec succès !"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

clean:
	@echo "Suppression des fichiers objets..."
	@rm -rf $(OBJ_DIR)

fclean: clean
	@echo "Suppression de l'exécutable $(NAME)..."
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re