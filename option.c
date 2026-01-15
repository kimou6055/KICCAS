#include "option.h"
#include <stdio.h>

void save_volume(int volume) {
  FILE *fichier = fopen("resources/file/volume.txt", "w");
  if (fichier) {
    fprintf(fichier, "%d", volume);
    fclose(fichier);
  }
}

int get_volume() {
  int volume = 64; // Default
  FILE *fichier = fopen("resources/file/volume.txt", "r");
  if (fichier) {
    fscanf(fichier, "%d", &volume);
    fclose(fichier);
  }
  return volume;
}

int afficher_option(GameContext *game) {
  // Load resources
  SDL_Texture *background =
      load_texture(game, "resources/image/optionback.png");
  SDL_Texture *volume_tex[4] = {
      load_texture(game, "resources/image/volume1.png"),
      load_texture(game, "resources/image/volume2.png"),
      load_texture(game, "resources/image/volume3.png"),
      load_texture(game, "resources/image/volume4.png")};
  SDL_Texture *full_tex[3] = {
      load_texture(game, "resources/image/button_full.png"),
      load_texture(game, "resources/image/button_full2.png"),
      load_texture(game, "resources/image/button_full1.png")};
  SDL_Texture *back_tex[3] = {
      load_texture(game, "resources/image/button_back.png"),
      load_texture(game, "resources/image/button_back2.png"),
      load_texture(game, "resources/image/button_back1.png")};

  Mix_Chunk *clickSound = Mix_LoadWAV("resources/sound/ClicDeSouris.wav");

  // Position setup
  SDL_Rect volume_pos = {450, 50, 300, 100};
  SDL_Rect full_pos = {100, 250, 300, 100};
  SDL_Rect back_pos = {100, 450, 300, 100};
  SDL_Rect text_pos = {100, 70, 0, 0}; // W/H will be set by render

  // Volume state mapping
  int vol_idx = 0;
  if (game->volume > 0)
    vol_idx = 1;
  if (game->volume > 40)
    vol_idx = 2;
  if (game->volume > 80)
    vol_idx = 3;

  int selected_button = 0; // 0: None, 1: Volume, 2: Fullscreen, 3: Back
  int return_code = 0;
  bool done = false;

  // Font setup
  TTF_Font *font = TTF_OpenFont("resources/font.ttf", 60);
  SDL_Color white = {255, 255, 255, 255};
  SDL_Color red = {255, 0, 0, 255};

  SDL_Event event;
  while (!done) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        return_code = 0; // Exit game
        done = true;
      } else if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_UP:
          selected_button--;
          if (selected_button < 1)
            selected_button = 3;
          break;
        case SDLK_DOWN:
          selected_button++;
          if (selected_button > 3)
            selected_button = 1;
          break;
        case SDLK_RETURN:
          if (selected_button == 2) { // Fullscreen
            game->fullscreen = !game->fullscreen;
            if (game->fullscreen)
              SDL_SetWindowFullscreen(game->window,
                                      SDL_WINDOW_FULLSCREEN_DESKTOP);
            else
              SDL_SetWindowFullscreen(game->window, 0);
          } else if (selected_button == 3) { // Back
            done = true;
            return_code = 1; // Back to menu
          }
          if (clickSound)
            Mix_PlayChannel(-1, clickSound, 0);
          break;
        case SDLK_KP_PLUS:
        case SDLK_PLUS:
        case SDLK_RIGHT:
          if (selected_button ==
              1) { // Only change volume if volume is selected
            game->volume += 10;
            if (game->volume > 128)
              game->volume = 128;
            Mix_VolumeMusic(game->volume);
            save_volume(game->volume);
            // Update vol_idx
            if (game->volume == 0)
              vol_idx = 0;
            else if (game->volume <= 42)
              vol_idx = 1;
            else if (game->volume <= 84)
              vol_idx = 2;
            else
              vol_idx = 3;
          }
          break;
        case SDLK_KP_MINUS:
        case SDLK_MINUS:
        case SDLK_LEFT:
          if (selected_button == 1) {
            game->volume -= 10;
            if (game->volume < 0)
              game->volume = 0;
            Mix_VolumeMusic(game->volume);
            save_volume(game->volume);
            // Update vol_idx
            if (game->volume == 0)
              vol_idx = 0;
            else if (game->volume <= 42)
              vol_idx = 1;
            else if (game->volume <= 84)
              vol_idx = 2;
            else
              vol_idx = 3;
          }
          break;
        }
      } else if (event.type == SDL_MOUSEMOTION) {
        int mx = event.motion.x;
        int my = event.motion.y;

        if (mx > volume_pos.x && mx < volume_pos.x + volume_pos.w &&
            my > volume_pos.y && my < volume_pos.y + volume_pos.h)
          selected_button = 1;
        else if (mx > full_pos.x && mx < full_pos.x + full_pos.w &&
                 my > full_pos.y && my < full_pos.y + full_pos.h)
          selected_button = 2;
        else if (mx > back_pos.x && mx < back_pos.x + back_pos.w &&
                 my > back_pos.y && my < back_pos.y + back_pos.h)
          selected_button = 3;
        else
          selected_button = 0;

      } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
          int mx = event.button.x;
          int my = event.button.y;

          // Check clicks on buttons
          if (mx > full_pos.x && mx < full_pos.x + full_pos.w &&
              my > full_pos.y && my < full_pos.y + full_pos.h) {
            // Fullscreen
            game->fullscreen = !game->fullscreen;
            if (game->fullscreen)
              SDL_SetWindowFullscreen(game->window,
                                      SDL_WINDOW_FULLSCREEN_DESKTOP);
            else
              SDL_SetWindowFullscreen(game->window, 0);
            if (clickSound)
              Mix_PlayChannel(-1, clickSound, 0);
          } else if (mx > back_pos.x && mx < back_pos.x + back_pos.w &&
                     my > back_pos.y && my < back_pos.y + back_pos.h) {
            // Back
            done = true;
            return_code = 1;
            if (clickSound)
              Mix_PlayChannel(-1, clickSound, 0);
          } else if (mx > volume_pos.x && mx < volume_pos.x + volume_pos.w &&
                     my > volume_pos.y && my < volume_pos.y + volume_pos.h) {
            // Cycle volume on click
            game->volume += 32;
            if (game->volume > 128)
              game->volume = 0; // Cycle
            Mix_VolumeMusic(game->volume);
            save_volume(game->volume);
            if (clickSound)
              Mix_PlayChannel(-1, clickSound, 0);

            // Update vol_idx
            if (game->volume == 0)
              vol_idx = 0;
            else if (game->volume <= 42)
              vol_idx = 1;
            else if (game->volume <= 84)
              vol_idx = 2;
            else
              vol_idx = 3;
          }
        }
      }
    }

    // Render
    SDL_RenderClear(game->renderer);
    if (background)
      SDL_RenderCopy(game->renderer, background, NULL, NULL);

    // Render buttons
    SDL_RenderCopy(game->renderer, volume_tex[vol_idx], NULL, &volume_pos);

    int full_state = (selected_button == 2) ? 1 : 0; // Simple hover effect
    if (game->fullscreen)
      full_state = 2; // Active state
    SDL_RenderCopy(game->renderer, full_tex[full_state], NULL, &full_pos);

    int back_state = (selected_button == 3) ? 1 : 0;
    SDL_RenderCopy(game->renderer, back_tex[back_state], NULL, &back_pos);

    // Render Text
    if (font) {
      SDL_Surface *tsurf = TTF_RenderText_Solid(
          font, "Volume", (selected_button == 1) ? red : white);
      if (tsurf) {
        SDL_Texture *ttex = SDL_CreateTextureFromSurface(game->renderer, tsurf);
        text_pos.w = tsurf->w;
        text_pos.h = tsurf->h;
        SDL_RenderCopy(game->renderer, ttex, NULL, &text_pos);
        SDL_DestroyTexture(ttex);
        SDL_FreeSurface(tsurf);
      }
    }

    SDL_RenderPresent(game->renderer);
  }

  // Cleanup resources for this scene
  SDL_DestroyTexture(background);
  for (int i = 0; i < 4; i++)
    SDL_DestroyTexture(volume_tex[i]);
  for (int i = 0; i < 3; i++)
    SDL_DestroyTexture(full_tex[i]);
  for (int i = 0; i < 3; i++)
    SDL_DestroyTexture(back_tex[i]);
  if (clickSound)
    Mix_FreeChunk(clickSound);
  if (font)
    TTF_CloseFont(font);

  return return_code;
}
