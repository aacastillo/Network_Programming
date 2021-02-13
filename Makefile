SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
BINS := $(patsubst %.c,%,$(SOURCES))

CC = gcc
CC_FLAGS = -w -g
LDFLAGS = -pthread

all: $(BINS)
	date

OBJ = $(patsubst %,%.o,$@)
%_threads: $(OBJECTS)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

%.o: %.c
	$(CC) -c $(CC_FLAGS) $< -o $@

clean:
	rm -f $(BINS) $(OBJECTS)
