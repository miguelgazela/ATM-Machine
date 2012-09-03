CC = gcc
CFLAGS =-c -Wall
LDFLAGS=-pthread -D-REENTRANT
SOURCES=Servidor.c ../includes/auxiliaryFunctions.c 
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=../bin/server

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *o ../bin/server
