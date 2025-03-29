all:
	gcc -Wall -Werror -Wextra -g -std=c2x -o run alog.c task.c main.c
clean:
	rm -rf run output.bin