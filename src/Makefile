CC = gcc
CFLAGS = -O3 -fPIC -Wall -Wextra -march=native -ffast-math
LDFLAGS = -shared -lm

TARGET = libsph.so
SOURCES = sph.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c sph.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
