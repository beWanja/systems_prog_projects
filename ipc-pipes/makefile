CC = gcc

CFLAGS = -g -Wall

TARGET = pipesort

SRCS = pipesort.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

# to build the target executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# to build object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# cleaning up build files
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
