CC = gcc
CFLAGS = -Wall -Werror -pthread 

SRCS = mysocket.c
OBJS = $(SRCS:.c=.o)
TARGET = libmsocket.a

all: $(TARGET)

$(TARGET): $(OBJS)
	ar rcs $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGET)
