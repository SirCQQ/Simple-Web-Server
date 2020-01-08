all:
	gcc srv.c -o server.out
	./server.out

test:
	gcc test.c -o test.out
	./test.out