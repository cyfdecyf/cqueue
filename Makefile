all: mpsc_test

#CFLAGS = -g -DDEBUG
CFLAGS = -O2 -DNDEBUG

mpsc_test: mpsc_test.o mpsc_q.o
	$(CC) $^ -lpthread -o $@

mpsc_q.o: mpsc_q.h

clean:
	@rm -f *.o test

