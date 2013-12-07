all: test

test:
	$(CC) -o test t/main.c
	./test
	rm -f test
