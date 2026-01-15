#include "intro.h"
#include <stdio.h>

void intro(GameContext *game) {
  SDL_Texture *logo = load_texture(game, "resources/image/logo.png");
  SDL_Texture *embleme = load_texture(game, "resources/image/embleme.png");

  if (!logo || !embleme) {
    printf("Failed to load intro images\n");
    return;
  }

  int alpha = 0;
  SDL_Event event;
  bool skip = false;

  // Fade in Logo
  while (alpha < 255 && !skip) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT ||
          (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
        skip = true;
      }
    }

    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderClear(game->renderer);

    SDL_SetTextureAlphaMod(logo, alpha);
    SDL_RenderCopy(game->renderer, logo, NULL, NULL);
    SDL_RenderPresent(game->renderer);

    alpha += 2;
    SDL_Delay(10);
  }

  SDL_Delay(500);

  // Fade out Logo
  alpha = 255;
  while (alpha > 0 && !skip) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT ||
          (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
        skip = true;
      }
    }
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderClear(game->renderer);
    SDL_SetTextureAlphaMod(logo, alpha);
    SDL_RenderCopy(game->renderer, logo, NULL, NULL);
    SDL_RenderPresent(game->renderer);
    alpha -= 5;
    SDL_Delay(10);
  }

  // Fade in Embleme
  alpha = 0;
  while (alpha < 255 && !skip) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT ||
          (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
        skip = true;
      }
    }

    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderClear(game->renderer);

    SDL_SetTextureAlphaMod(embleme, alpha);
    SDL_RenderCopy(game->renderer, embleme, NULL, NULL);
    SDL_RenderPresent(game->renderer);

    alpha += 2;
    SDL_Delay(10);
  }

  SDL_Delay(500);

  SDL_DestroyTexture(logo);
  SDL_DestroyTexture(embleme);
}
