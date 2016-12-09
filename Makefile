CC = gcc
CFLAGS = -O0 -Wall -I. -lpthread -lrt -DLOGGING_LEVEL=LOG_LEVEL_NONE
RM = rm -f

DEPS = includes/utils.h includes/wfqueue.h includes/blqueue.h includes/logger.h includes/list.h
OBJ = logger.o wfqueue.o blqueue.o  main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	$(RM) $(OBJ) *.o main
