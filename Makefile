compile: hhush.c
	@gcc -Wall -Wextra -std=c99 hhush.c -g -o hhush

clean:
	@rm -f ./hhush .hhush.histfile

valgrind: compile
	valgrind -v --track-origins=yes --leak-check=full --show-reachable=yes ./hhush

run: compile
	@./hhush
