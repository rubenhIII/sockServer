SOURCES = aesdsocket.c
OBJS = $(SOURCES:.c=.o)
TARGET = aesdsocket

CROSS_COMPILE =#aarch64-none-linux-gnu-#Cross-Compile Toolchain 
CC = gcc
OPTS = -Wall # flags
LIBS = #-lm # libraries

default: all
all: $(TARGET)
$(TARGET): $(OBJS)
	$(CROSS_COMPILE)$(CC) -o $(TARGET) $(OBJS) $(LIBS)

%.o: %.c
	$(CROSS_COMPILE)$(CC) $(OPTS) -c $< -o $@
clean:
	rm -f $(OBJS) $(TARGET)
