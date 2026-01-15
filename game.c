#include "game.h"
#include <stdio.h>

bool init_game(GameContext *game) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return false;
  }

  if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
    printf("SDL_image could not initialize! IMG_Error: %s\n", IMG_GetError());
    return false;
  }

  if (TTF_Init() == -1) {
    printf("SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());
    return false;
  }

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    printf("SDL_mixer could not initialize! Mix_Error: %s\n", Mix_GetError());
    return false;
  }

  game->window = SDL_CreateWindow(
      "KICCAS", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
      SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if (!game->window) {
    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return false;
  }

  game->renderer = SDL_CreateRenderer(
      game->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!game->renderer) {
    printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
    return false;
  }
  SDL_SetRenderDrawBlendMode(game->renderer, SDL_BLENDMODE_BLEND);

  game->running = true;
  game->volume = 64; // Default volume
  game->fullscreen = false;
  game->font = NULL;
  game->bgMusic = NULL;

  return true;
}

void close_game(GameContext *game) {
  if (game->font) {
    TTF_CloseFont(game->font);
    game->font = NULL;
  }
  if (game->bgMusic) {
    Mix_FreeMusic(game->bgMusic);
    game->bgMusic = NULL;
  }

  if (game->renderer) {
    SDL_DestroyRenderer(game->renderer);
    game->renderer = NULL;
  }
  if (game->window) {
    SDL_DestroyWindow(game->window);
    game->window = NULL;
  }

  Mix_Quit();
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}

SDL_Texture *load_texture(GameContext *game, const char *path) {
  SDL_Surface *surface = IMG_Load(path);
  if (!surface) {
    printf("Unable to load image %s! IMG_Error: %s\n", path, IMG_GetError());
    return NULL;
  }
  SDL_Texture *texture = SDL_CreateTextureFromSurface(game->renderer, surface);
  if (!texture) {
    printf("Unable to create texture from %s! SDL_Error: %s\n", path,
           SDL_GetError());
  }
  SDL_FreeSurface(surface);
  return texture;
}
