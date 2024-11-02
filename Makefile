# Makefile rally_marciano.c

CC = gcc
CFLAGS = -Wall -pthread
TARGET = rally_marciano

all: $(TARGET)

$(TARGET): rally_marciano.o
	$(CC) $(CFLAGS) -o $(TARGET) rally_marciano.o

rally_marciano.o: rally_marciano.c
	$(CC) $(CFLAGS) -c rally_marciano.c

clean:
	rm -f *.o $(TARGET)