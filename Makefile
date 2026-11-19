CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2

all: calculadora.exe

calculadora.exe: calculadora.c
	$(CC) $(CFLAGS) calculadora.c -o calculadora.exe -mwindows -luser32 -lgdi32

clean:
	rm -f calculadora.exe

.PHONY: all clean