CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
SRCS = main.c sorting.c
OBJS = $(SRCS:.c=.o)
TARGET = test_runner
all: $(TARGET)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
test: all
	./$(TARGET)
clean:
	rm -f $(OBJS) $(TARGET)
.PHONY: all test clean
