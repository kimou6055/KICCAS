#include "levels.h"
#include <math.h>
#include <stdio.h>

// --- ASSET PATHS ---
static const char *BG_PATHS[] = {
    "", "resources/image/niveau1.png", "resources/image/background2.jpg",
    "resources/image/cave background.png", "resources/image/map1.png"};

static const char *MASK_PATHS[] = {
    "", "resources/image/map1_masked.png", "resources/image/map2_masked.png",
    "resources/image/map3_masked.png", "resources/image/map1_masked.png"};

// --- HELPER: Load Anim ---
static void load_anim(GameContext *game, SDL_Texture **frames,
                      const char *pattern, int count) {
  char buffer[128];
  for (int i = 0; i < count; i++) {
    sprintf(buffer, pattern, i);
    frames[i] = load_texture(game, buffer);
  }
}

// --- HELPER: Pixel Reader ---
static Uint32 get_pixel(SDL_Surface *surface, int x, int y) {
  if (!surface || x < 0 || y < 0 || x >= surface->w || y >= surface->h)
    return 0;
  int bpp = surface->format->BytesPerPixel;
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

  switch (bpp) {
  case 1:
    return *p;
  case 2:
    return *(Uint16 *)p;
  case 3:
    return (SDL_BYTEORDER == SDL_BIG_ENDIAN) ? (p[0] << 16 | p[1] << 8 | p[2])
                                             : (p[0] | p[1] << 8 | p[2] << 16);
  case 4:
    return *(Uint32 *)p;
  default:
    return 0;
  }
}

static bool is_solid(SDL_Surface *mask, int x, int y) {
  if (!mask)
    return false;

  Uint32 pixel = get_pixel(mask, x, y);
  SDL_Color color;
  SDL_GetRGB(pixel, mask->format, &color.r, &color.g, &color.b);
  return (color.r == 0 && color.g == 0 && color.b == 0); // Black is Wall
}

// --- PHYSICS: Check Collision (SCALED) ---
static bool check_collision(LevelMap *map, SDL_Rect box) {
  int step = 5;

// Helper for point check with scale
#define IS_SOLID_SCALED(mx, my)                                                \
  is_solid(map->mask, (int)((mx) * map->scale_x), (int)((my) * map->scale_y))

  // Bottom
  for (int x = box.x; x < box.x + box.w; x += step)
    if (IS_SOLID_SCALED(x, box.y + box.h))
      return true;
  if (IS_SOLID_SCALED(box.x + box.w, box.y + box.h))
    return true;

  // Top
  for (int x = box.x; x < box.x + box.w; x += step)
    if (IS_SOLID_SCALED(x, box.y))
      return true;
  if (IS_SOLID_SCALED(box.x + box.w, box.y))
    return true;

  // Left
  for (int y = box.y; y < box.y + box.h; y += step)
    if (IS_SOLID_SCALED(box.x, y))
      return true;

  // Right
  for (int y = box.y; y < box.y + box.h; y += step)
    if (IS_SOLID_SCALED(box.x + box.w, y))
      return true;

#undef IS_SOLID_SCALED
  return false;
}

// --- HUD RENDERING ---
static void render_hud(GameContext *game, Player *p, int level_id,
                       int time_left, TTF_Font *font) {
  if (!font)
    return;

  SDL_Color white = {255, 255, 255, 255};
  char buffer[64];

  // Score
  sprintf(buffer, "MARIO\n%06d", p->score);
  SDL_Surface *surf = TTF_RenderText_Blended_Wrapped(font, buffer, white, 0);
  if (surf) {
    SDL_Texture *tex = SDL_CreateTextureFromSurface(game->renderer, surf);
    SDL_Rect r = {20, 10, surf->w, surf->h};
    SDL_RenderCopy(game->renderer, tex, NULL, &r);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
  }

  // Level
  sprintf(buffer, "WORLD\n1-%d", level_id);
  surf = TTF_RenderText_Blended_Wrapped(font, buffer, white, 0);
  if (surf) {
    SDL_Texture *tex = SDL_CreateTextureFromSurface(game->renderer, surf);
    SDL_Rect r = {280, 10, surf->w, surf->h};
    SDL_RenderCopy(game->renderer, tex, NULL, &r);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
  }

  // Time
  sprintf(buffer, "TIME\n%03d", time_left);
  surf = TTF_RenderText_Blended_Wrapped(font, buffer, white, 0);
  if (surf) {
    SDL_Texture *tex = SDL_CreateTextureFromSurface(game->renderer, surf);
    SDL_Rect r = {550, 10, surf->w, surf->h};
    SDL_RenderCopy(game->renderer, tex, NULL, &r);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
  }

  // Lives
  for (int i = 0; i < p->lives; i++) {
    SDL_Rect heart_pos = {20 + (30 * i), 60, 25, 25};
    if (p->hearts[0])
      SDL_RenderCopy(game->renderer, p->hearts[0], NULL, &heart_pos);
  }
}

// --- INITIALIZATION ---
static bool init_level(GameContext *game, int level_id, Player *p,
                       Enemy enemies[], int *enemy_count, LevelMap *map) {
  if (level_id < 1 || level_id > 4)
    level_id = 1;

  map->texture = load_texture(game, BG_PATHS[level_id]);
  map->mask = IMG_Load(MASK_PATHS[level_id]);

  if (!map->texture || !map->mask) {
    printf("Failed to load level %d assets.\n", level_id);
    return false;
  }

  // Get Map Dimensions
  SDL_QueryTexture(map->texture, NULL, NULL, &map->width, &map->height);

  // CALCULATE SCALE FACTORS
  map->scale_x = (float)map->mask->w / (float)map->width;
  map->scale_y = (float)map->mask->h / (float)map->height;

  printf("DEBUG Level %d: Map[%dx%d] Mask[%dx%d] -> Scale[%.2fx%.2f]\n",
         level_id, map->width, map->height, map->mask->w, map->mask->h,
         map->scale_x, map->scale_y);

  // Set camera to Logic Size (Retro Zoom)
  map->camera = (SDL_Rect){0, 0, 640, 360};

  // Init Player
  load_anim(game, p->anim_right, "resources/image/RW%d.png", 4);
  load_anim(game, p->anim_left, "resources/image/LW%d.png", 4);

  p->hearts[0] = load_texture(game, "resources/image/v4.png");

  p->x = 50;
  p->y = 300;
  p->vx = 0;
  p->vy = 0;

  p->rect = (SDL_Rect){0, 0, 50, 70};
  if (p->anim_right[0]) {
    int w, h;
    SDL_QueryTexture(p->anim_right[0], NULL, NULL, &w, &h);
    p->rect.w = w - 25;
    p->rect.h = h - 10;
  }

  p->direction = 0;
  p->frame = 0;
  p->lives = 3;
  p->on_ground = false;

  // Init Enemies
  *enemy_count = 5 + level_id;
  for (int i = 0; i < *enemy_count; i++) {
    load_anim(game, enemies[i].anim_right, "resources/image/ER%d.png", 4);
    load_anim(game, enemies[i].anim_left, "resources/image/EL%d.png", 4);
    enemies[i].x = 800 + (i * 500);
    enemies[i].y = 50;
    enemies[i].vx = 2.0f;
    enemies[i].vy = 0;
    enemies[i].active = true;
    enemies[i].rect = (SDL_Rect){0, 0, 60, 70};
    enemies[i].type = ENEMY_PATROL;

    if (enemies[i].anim_left[0]) {
      SDL_QueryTexture(enemies[i].anim_left[0], NULL, NULL, &enemies[i].rect.w,
                       &enemies[i].rect.h);
      enemies[i].rect.w -= 20;
      enemies[i].rect.h -= 10;
    }
  }

  return true;
}

// --- UPDATE ---
static void update_physics(Player *p, LevelMap *map, const Uint8 *keys,
                           Mix_Chunk *sfx_jump, Enemy enemies[],
                           int enemy_count) {
  // Horizontal
  float target_vx = 0;
  if (keys[SDL_SCANCODE_RIGHT]) {
    target_vx = MAX_SPEED;
    p->direction = 0;
  } else if (keys[SDL_SCANCODE_LEFT]) {
    target_vx = -MAX_SPEED;
    p->direction = 1;
  }

  // Friction/Accel
  if (target_vx > p->vx) {
    p->vx += ACCELERATION;
    if (p->vx > target_vx)
      p->vx = target_vx;
  } else if (target_vx < p->vx) {
    p->vx -= ACCELERATION;
    if (p->vx < target_vx)
      p->vx = target_vx;
  } else {
    if (p->vx > 0) {
      p->vx -= FRICTION;
      if (p->vx < 0)
        p->vx = 0;
    } else if (p->vx < 0) {
      p->vx += FRICTION;
      if (p->vx > 0)
        p->vx = 0;
    }
  }

  // Vertical
  if (keys[SDL_SCANCODE_SPACE] && p->on_ground) {
    p->vy = JUMP_FORCE;
    p->on_ground = false;
    p->is_jumping = true;
    if (sfx_jump)
      Mix_PlayChannel(-1, sfx_jump, 0);
  }

  if (!keys[SDL_SCANCODE_SPACE] && p->vy < 0) {
    p->vy *= 0.5f;
  }

  p->vy += GRAVITY;
  if (p->vy > MAX_FALL_SPEED)
    p->vy = MAX_FALL_SPEED;

  // Collision X
  p->x += p->vx;
  p->rect.x = (int)p->x;
  p->rect.y = (int)p->y;

  if (check_collision(map, p->rect)) {
    p->x -= p->vx;
    p->vx = 0;
  }

  if (p->x < 0)
    p->x = 0;
  if (p->x > map->width - p->rect.w)
    p->x = map->width - p->rect.w;

  // Collision Y
  p->y += p->vy;
  p->rect.x = (int)p->x;
  p->rect.y = (int)p->y;

  p->on_ground = false;

  if (check_collision(map, p->rect)) {
    if (p->vy > 0) { // Landing
      p->on_ground = true;
      while (check_collision(map, p->rect)) {
        p->y -= 1.0f;
        p->rect.y = (int)p->y;
      }
      p->vy = 0;
    } else if (p->vy < 0) { // Ceiling
      while (check_collision(map, p->rect)) {
        p->y += 1.0f;
        p->rect.y = (int)p->y;
      }
      p->vy = 0;
    }
  }

  if (p->y > map->height) {
    p->lives--;
    p->x = 100;
    p->y = 100;
    p->vy = 0;
  }

  // Enemy Interactions
  for (int i = 0; i < enemy_count; i++) {
    if (!enemies[i].active)
      continue;

    if (SDL_HasIntersection(&p->rect, &enemies[i].rect)) {
      bool is_stomp = (p->vy > 0) && (p->y + p->rect.h / 2 < enemies[i].y);

      if (is_stomp) {
        enemies[i].active = false;
        p->vy = JUMP_FORCE * 0.5f;
        p->score += 100;
        if (sfx_jump)
          Mix_PlayChannel(-1, sfx_jump, 0);
      } else {
        p->lives--;
        p->vy = JUMP_FORCE * 0.8f;
        p->vx = (p->x < enemies[i].x) ? -5.0f : 5.0f;
      }
    }

    enemies[i].vy += GRAVITY;
    enemies[i].y += enemies[i].vy;
    enemies[i].rect.y = (int)enemies[i].y;

    if (check_collision(map, enemies[i].rect)) {
      if (enemies[i].vy > 0) {
        enemies[i].vy = 0;
        while (check_collision(map, enemies[i].rect)) {
          enemies[i].y -= 1.0f;
          enemies[i].rect.y = (int)enemies[i].y;
        }
      }
    }

    enemies[i].x += enemies[i].vx;
    enemies[i].rect.x = (int)enemies[i].x;

    if (check_collision(map, enemies[i].rect)) {
      enemies[i].vx *= -1;
      enemies[i].x += enemies[i].vx;
    }
  }

  // Animation
  if (fabs(p->vx) > 0.5f) {
    p->anim_timer++;
    if (p->anim_timer > 5) {
      p->frame = (p->frame + 1) % 4;
      p->anim_timer = 0;
    }
  } else {
    p->frame = 0;
  }
}

static void update_camera(LevelMap *map, Player *p) {
  int center_x = 640 / 2;
  int center_y = 360 / 2;

  int target_x = (int)p->x - center_x;
  int target_y = (int)p->y - center_y;

  map->camera.x += (target_x - map->camera.x) * 0.1f;
  map->camera.y += (target_y - map->camera.y) * 0.1f;

  if (map->camera.x < 0)
    map->camera.x = 0;
  if (map->camera.x > map->width - map->camera.w)
    map->camera.x = map->width - map->camera.w;

  if (map->camera.y < 0)
    map->camera.y = 0;
  if (map->camera.y > map->height - map->camera.h)
    map->camera.y = map->height - map->camera.h;
}

// --- MAIN LOOP ---
int play_level(GameContext *game, int level_id) {

  // Set Retro Resolution
  SDL_RenderSetLogicalSize(game->renderer, 640, 360);

  Player p;
  Enemy enemies[MAX_ENEMIES];
  int enemy_count = 0;
  LevelMap map; // Local struct

  if (!init_level(game, level_id, &p, enemies, &enemy_count, &map)) {
    SDL_RenderSetLogicalSize(game->renderer, 0, 0);
    return 0;
  }

  Mix_Music *bgm = Mix_LoadMUS("resources/sound/music.mp3");
  Mix_Chunk *sfx_jump = Mix_LoadWAV("resources/sound/ClicDeSouris.wav");
  if (bgm)
    Mix_PlayMusic(bgm, -1);

  TTF_Font *font = TTF_OpenFont("resources/font.ttf", 24);

  bool running = true;
  int next_action = 0;
  SDL_Event event;
  const Uint8 *keys = SDL_GetKeyboardState(NULL);

  int game_time = 400 * 60;

  while (running && game->running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        game->running = false;
        running = false;
      } else if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          running = false;
        }
      }
    }

    if (game_time > 0)
      game_time--;

    update_physics(&p, &map, keys, sfx_jump, enemies, enemy_count);
    update_camera(&map, &p);

    if (p.x > map.width - 200) {
      running = false;
      next_action = level_id + 1;
    }

    if (p.lives <= 0) {
      running = false;
      next_action = 0;
    }

    // Render
    SDL_RenderClear(game->renderer);
    // Correctly using dot operator for local struct 'map'
    SDL_RenderCopy(game->renderer, map.texture, &map.camera, NULL);

    SDL_Rect rel_p = p.rect;
    rel_p.x -= map.camera.x;
    rel_p.y -= map.camera.y;

    SDL_Texture *tex =
        (p.direction == 0) ? p.anim_right[p.frame] : p.anim_left[p.frame];

    if (tex) {
      SDL_RenderCopy(game->renderer, tex, NULL, &rel_p);
    }

    for (int i = 0; i < enemy_count; i++) {
      if (enemies[i].active) {
        SDL_Rect rel_e = enemies[i].rect;
        rel_e.x = (int)enemies[i].x - map.camera.x;
        rel_e.y = (int)enemies[i].y - map.camera.y;
        SDL_Texture *etex = (enemies[i].vx > 0) ? enemies[i].anim_right[0]
                                                : enemies[i].anim_left[0];
        SDL_RenderCopy(game->renderer, etex, NULL, &rel_e);
      }
    }

    render_hud(game, &p, level_id, game_time / 60, font);

    SDL_RenderPresent(game->renderer);
    SDL_Delay(16);
  }

  // Cleanup
  if (map.texture)
    SDL_DestroyTexture(map.texture);
  if (map.mask)
    SDL_FreeSurface(map.mask);
  if (bgm)
    Mix_FreeMusic(bgm);
  if (sfx_jump)
    Mix_FreeChunk(sfx_jump);
  if (font)
    TTF_CloseFont(font);

  SDL_RenderSetLogicalSize(game->renderer, 0, 0);
  return next_action;
}
