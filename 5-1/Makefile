EXEC_NAME=large_test
CFLAGS=-Wall -Werror -Wfatal-errors -g

all:
	gcc main.c -o $(EXEC_NAME) $(CFLAGS) -D_FILE_OFFSET_BITS=64

no_large_file:
	gcc main.c -o $(EXEC_NAME) $(CFLAGS)

clean:
	rm -rf ./$(EXEC_NAME) *.o
