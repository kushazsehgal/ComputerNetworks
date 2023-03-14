CC = gcc
CFLAGS = -Wall -Werror -pthread

SERVER_SRCS = mysocket.c test_server.c
CLIENT_SRCS = mysocket.c test_client.c

SERVER_OBJS = $(SERVER_SRCS:.c=.o)
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)

all: test_server test_client

test_server: $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

test_client: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o test_server test_client

kill:
	sudo fuser -k 8000/tcp
