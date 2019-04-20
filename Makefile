CC=gcc
CFFLAGS = -c -Wall -O3
LDFLAGS = -lm -lpthread

EXECUTABLE = ppm_converter
SOURCES = main.c pam.c pam_converters.c sobel.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = pam.h pam_converters.h sobel.h


all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXECUTABLE) LFLAGS

main.o: main.c
	$(CC) $(CFLAGS) main.c

pam.o: pam.c
	$(CC) $(CFLAGS) pam.c

pam_converters.o: pam_converters.c
	$(CC) $(CFLAGS) pam_converters.c

sobel.o: sobel.c
	$(CC) $(CFLAGS) sobel.c

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)