# =========================================================
# Makefile – University Lab Slot Allocation System
# =========================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -Iinclude
TARGET  = lab_allocator
SRCS    = src/main.c src/lab.c src/room.c src/scheduler.c src/utils.c
OBJS    = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile each .c to .o
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Remove compiled files
clean:
	rm -f src/*.o $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
