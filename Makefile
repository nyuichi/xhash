all: test

test:
	$(CC) -Wall -Wextra -Werror -o test t/main.c
	./test
	rm -f test
