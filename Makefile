all:
	gcc -Wall -Werror -Wextra -g -std=c2x -o run thirdparty/alog.c src/task.c src/common.c src/main.c
clean:
	rm -rf run output.bin