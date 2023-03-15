CC = gcc
CFLAGS = -Wall -Werror 
LIB_FLAGS = -pthread
LIB_SRCS = mysocket.c
LIB_HDRS = mysocket.h
TEST_SRC = test_client.c \
		test_server.c
TEST_OBJS = $(TEST_SRC:.c=.o)
LIB_OBJS = $(LIB_SRCS:.c=.o)
TEST_EXE = $(TEST_SRC:.c=.out)
TARGET = libmsocket.a

.PHONY: all lib clean

all: $(TEST_EXE)
lib: $(TARGET)
$(TARGET): $(LIB_OBJS) 
	ar rcs $@ $<
$(TEST_OBJS): %.o : %.c $(LIB_HDRS)
	$(CC) $(CFLAGS) -c $< -o $@
$(LIB_OBJS): %.o : %.c $(LIB_HDRS)
	$(CC) $(CFLAGS) $(LIB_FLAGS) -c $< -o $@
$(TEST_EXE): %.out : %.o $(TARGET) $(LIB_HDRS)
	$(CC) $(CFLAGS) $< -L. -lmsocket -o $@ 

clean:
	rm -f *.o 
distclean:
	rm -f *.o $(TARGET) $(TEST_EXE)