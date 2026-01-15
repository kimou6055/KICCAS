# Makefile for SDL2 (MacOS)
CC = gcc
# Include paths for Homebrew (Apple Silicon & Intel)
INCLUDES = -I/opt/homebrew/include -I/usr/local/include
CFLAGS = -Wall -g $(INCLUDES)

# Library paths for Homebrew
LIBS = -L/opt/homebrew/lib -L/usr/local/lib -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf

OBJ = main.o game.o intro.o fonctions.o option.o puissance4.o levels.o

all: game

game: $(OBJ)
	$(CC) -o $@ $^ $(LIBS)
	
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o game
