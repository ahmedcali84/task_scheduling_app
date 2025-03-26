all:
	gcc -Wall -Werror -Wextra -g -std=c17 -o run main.c
clean:
	rm -rf run
