CC = gcc
CFLAGS =-c -Wall
LDFLAGS=
SOURCES=Cliente.c ../includes/auxiliaryFunctions.c 
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=../bin/client

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *o ../bin/client
