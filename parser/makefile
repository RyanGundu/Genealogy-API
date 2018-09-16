CC=gcc
CFLAGS= -std=c11 -Iinclude

#UNAME Shell Variable
UNAME_S := $(shell uname -s)

# OS X Dependant Variables
ifeq ($(UNAME_S), Darwin)
    LIB := ../sharedLib.dylib
# GNU/Linux Depedant Variables
else ifeq ($(UNAME_S), Linux)
    LIB := ../sharedLib.so
endif

$(LIB): LinkedListAPI.o GEDCOMparser.o GEDCOMutilities.o
	$(CC) -shared -o $(LIB) LinkedListAPI.o GEDCOMparser.o GEDCOMutilities.o

LinkedListAPI.o: src/LinkedListAPI.c
	$(CC) $(CFLAGS) -c -fpic src/LinkedListAPI.c

GEDCOMparser.o: src/GEDCOMparser.c
	$(CC) $(CFLAGS) -c -fpic src/GEDCOMparser.c

GEDCOMutilities.o: src/GEDCOMutilities.c
	$(CC) $(CFLAGS) -c -fpic src/GEDCOMutilities.c

clean:
	rm $(LIB) *.o

	
