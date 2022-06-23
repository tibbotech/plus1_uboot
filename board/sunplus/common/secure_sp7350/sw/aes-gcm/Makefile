CC=gcc
CFLAGS=-c -Wall
CFLAGS+= -ffunction-sections -fdata-sections
LDFLAGS= -Wl,--gc-sections,--print-gc-sections
#SOURCES=aes.c cipher.c cipher_wrap.c gcm.c main.c utils.c
SOURCES=aes.c cipher.c cipher_wrap.c gcm.c main.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=aes-gcm

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE)

