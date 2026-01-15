#include "fonctions.h"
#include <stdio.h>

void init_cursor(GameContext *game, Cursor *c, int x, int y) {
  c->texture = load_texture(game, "resources/image/galaxy.png");
  c->pos.x = x;
  c->pos.y = y;
  c->pos.w = 60;
  c->pos.h = 65;
  c->show = true;
  c->frame = 0;

  // Init animation frames
  c->animation[0].x = 0;
  c->animation[0].y = 0;
  c->animation[0].w = 60;
  c->animation[0].h = 65;

  for (int i = 1; i < 9; i++) {
    c->animation[i].w = 60;
    c->animation[i].h = 65;
    c->animation[i].x = c->animation[i].w + c->animation[i - 1].x;
    c->animation[i].y = 0;
  }
}

void update_cursor(Cursor *c, int x, int y) {
  if (!c->show)
    return;

  // Animation
  c->frame++;
  if (c->frame >= 9)
    c->frame = 0;

  // Smooth movement logic (simple lerp-like or step)
  if (c->pos.x < x)
    c->pos.x += 5;
  if (c->pos.x > x)
    c->pos.x -= 5;
  if (c->pos.y < y)
    c->pos.y += 5;
  if (c->pos.y > y)
    c->pos.y -= 5;
}

void draw_cursor(GameContext *game, Cursor *c) {
  if (c->show && c->texture) {
    SDL_RenderCopy(game->renderer, c->texture, &c->animation[c->frame],
                   &c->pos);
  }
}

int afficher_menu(GameContext *game) {
  SDL_Texture *background = load_texture(game, "resources/image/menuback.png");

  SDL_Texture *start[2] = {
      load_texture(game, "resources/image/button_start.png"),
      load_texture(game, "resources/image/button_start2.png")};
  SDL_Texture *setting[2] = {
      load_texture(game, "resources/image/button_settings.png"),
      load_texture(game, "resources/image/button_settings2.png")};
  SDL_Texture *credit[2] = {
      load_texture(game, "resources/image/button_credits.png"),
      load_texture(game, "resources/image/button_credits2.png")};
  SDL_Texture *quit[2] = {
      load_texture(game, "resources/image/button_quit.png"),
      load_texture(game, "resources/image/button_quit2.png")};

  SDL_Rect start_pos = {150, 100, 333, 119};
  SDL_Rect setting_pos = {150, 250, 333, 119};
  SDL_Rect credit_pos = {150, 400, 333, 119};
  SDL_Rect quit_pos = {150, 550, 333, 119};

  Mix_Chunk *clickSound = Mix_LoadWAV("resources/sound/ClicDeSouris.wav");
  Mix_Chunk *hoverSound = Mix_LoadWAV("resources/sound/ClicDeSouris2.wav");

  if (game->bgMusic == NULL) {
    game->bgMusic = Mix_LoadMUS("resources/sound/music.mp3");
    Mix_PlayMusic(game->bgMusic, -1);
  }
  Mix_VolumeMusic(game->volume);

  Cursor cursor;
  int mx = 700, my = 500;
  init_cursor(game, &cursor, mx, my);

  int selected = 0; // 0 none, 1 start, 2 setting, 3 credit, 4 quit
  int return_code = -1;
  bool done = false;

  SDL_Event event;
  while (!done) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        return_code = 0;
        done = true;
      } else if (event.type == SDL_KEYDOWN) {
        cursor.show = false;
        switch (event.key.keysym.sym) {
        case SDLK_UP:
          selected--;
          if (selected < 1)
            selected = 4;
          Mix_PlayChannel(-1, clickSound, 0);
          break;
        case SDLK_DOWN:
          selected++;
          if (selected > 4)
            selected = 1;
          Mix_PlayChannel(-1, clickSound, 0);
          break;
        case SDLK_RETURN:
          if (selected == 1)
            return_code = 1;
          else if (selected == 2)
            return_code = 2;
          else if (selected == 3)
            return_code = 3;
          else if (selected == 4)
            return_code = 0;
          if (return_code != -1)
            done = true;
          Mix_PlayChannel(-1, clickSound, 0);
          SDL_Delay(200);
          break;
        case SDLK_q:
        case SDLK_ESCAPE:
          return_code = 0;
          done = true;
          break;
        }
      } else if (event.type == SDL_MOUSEMOTION) {
        cursor.show = true;
        SDL_GetMouseState(&mx, &my);

        int prev_selected = selected;
        selected = 0;
        if (mx > start_pos.x && mx < start_pos.x + start_pos.w &&
            my > start_pos.y && my < start_pos.y + start_pos.h)
          selected = 1;
        else if (mx > setting_pos.x && mx < setting_pos.x + setting_pos.w &&
                 my > setting_pos.y && my < setting_pos.y + setting_pos.h)
          selected = 2;
        else if (mx > credit_pos.x && mx < credit_pos.x + credit_pos.w &&
                 my > credit_pos.y && my < credit_pos.y + credit_pos.h)
          selected = 3;
        else if (mx > quit_pos.x && mx < quit_pos.x + quit_pos.w &&
                 my > quit_pos.y && my < quit_pos.y + quit_pos.h)
          selected = 4;

        if (selected != 0 && selected != prev_selected) {
          Mix_PlayChannel(-1, hoverSound, 0);
        }
      } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
          if (selected == 1)
            return_code = 1;
          else if (selected == 2)
            return_code = 2;
          else if (selected == 3)
            return_code =
                3; // Credits not fully implemented so just stay or return code
          else if (selected == 4)
            return_code = 0;

          if (selected != 0) {
            Mix_PlayChannel(-1, clickSound, 0);
            SDL_Delay(200);
            done = true;
          }
        }
      }
    }

    update_cursor(&cursor, mx, my);

    // Render
    SDL_RenderClear(game->renderer);
    if (background)
      SDL_RenderCopy(game->renderer, background, NULL, NULL);

    SDL_RenderCopy(game->renderer, start[selected == 1 ? 1 : 0], NULL,
                   &start_pos);
    SDL_RenderCopy(game->renderer, setting[selected == 2 ? 1 : 0], NULL,
                   &setting_pos);
    SDL_RenderCopy(game->renderer, credit[selected == 3 ? 1 : 0], NULL,
                   &credit_pos);
    SDL_RenderCopy(game->renderer, quit[selected == 4 ? 1 : 0], NULL,
                   &quit_pos);

    draw_cursor(game, &cursor);

    SDL_RenderPresent(game->renderer);
  }

  // Cleanup
  SDL_DestroyTexture(background);
  for (int i = 0; i < 2; i++)
    SDL_DestroyTexture(start[i]);
  for (int i = 0; i < 2; i++)
    SDL_DestroyTexture(setting[i]);
  for (int i = 0; i < 2; i++)
    SDL_DestroyTexture(credit[i]);
  for (int i = 0; i < 2; i++)
    SDL_DestroyTexture(quit[i]);
  SDL_DestroyTexture(cursor.texture);
  Mix_FreeChunk(clickSound);
  Mix_FreeChunk(hoverSound);

  return return_code;
}
