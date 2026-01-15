#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

#define SCREEN_WIDTH 1366
#define SCREEN_HEIGHT 768

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  TTF_Font *font;
  Mix_Music *bgMusic;
  bool running;
  int volume; // 0-128
  bool fullscreen;
} GameContext;

// Initialize SDL2, Window, Renderer, Mixer, TTF
bool init_game(GameContext *game);

// Clean up resources
void close_game(GameContext *game);

// Utility: Load a texture from file
SDL_Texture *load_texture(GameContext *game, const char *path);

#endif
