CC = gcc
CFLAGS = -O0 -Wall -I. -lpthread -lrt -DLOGGING_LEVEL=LOG_LEVEL_INFO
RM = rm -f

DEPS = includes/utils.h includes/wfqueue.h includes/logger.h includes/list.h
OBJ = logger.o wfqueue.o main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	$(RM) $(OBJ) *.o main
