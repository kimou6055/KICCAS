#ifndef FONCTIONS_H_INCLUDED
#define FONCTIONS_H_INCLUDED

#include "game.h"

typedef struct {
  SDL_Texture *texture;
  SDL_Rect pos;
  SDL_Rect animation[9]; // Increased to 9 to match logic
  int frame;
  bool show;
} Cursor;

// Initialize cursor
void init_cursor(GameContext *game, Cursor *c, int x, int y);

// Update/Draw cursor
void update_cursor(Cursor *c, int x, int y);
void draw_cursor(GameContext *game, Cursor *c);

// Main Menu Loop
// Returns: 0=Quit, 1=Play, 2=Options, 3=Credits
int afficher_menu(GameContext *game);

#endif
