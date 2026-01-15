#include "level1.h"
#include <math.h>
#include <stdio.h>

#define GRAVITY 0.5f
#define JUMP_FORCE -12.0f
#define MAX_FALL_SPEED 10.0f

// Helper to load animation frames
void load_anim(GameContext *game, SDL_Texture **frames, const char *pattern,
               int count) {
  char buffer[128];
  for (int i = 0; i < count; i++) {
    sprintf(buffer, pattern, i);
    frames[i] = load_texture(game, buffer);
  }
}

// ---------------------------------------------------------
// COLLISION UTILS
// ---------------------------------------------------------

static Uint32 get_pixel(SDL_Surface *surface, int x, int y) {
  int bpp = surface->format->BytesPerPixel;
  /* Here p is the address to the pixel we want to retrieve */
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

  switch (bpp) {
  case 1:
    return *p;
  case 2:
    return *(Uint16 *)p;
  case 3:
    if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
      return p[0] << 16 | p[1] << 8 | p[2];
    else
      return p[0] | p[1] << 8 | p[2] << 16;
  case 4:
    return *(Uint32 *)p;
  default:
    return 0;
  }
}

static SDL_Color get_pixel_color(SDL_Surface *surface, int x, int y) {
  if (!surface || x < 0 || y < 0 || x >= surface->w || y >= surface->h) {
    return (SDL_Color){0, 0, 0, 0};
  }

  Uint32 pixel = get_pixel(surface, x, y);
  SDL_Color color;
  SDL_GetRGB(pixel, surface->format, &color.r, &color.g, &color.b);
  return color;
}

// Check if a specific point collides
bool is_colliding(SDL_Surface *mask, int x, int y) {
  SDL_Color color = get_pixel_color(mask, x, y);
  // Assuming BLACK is collision (0,0,0)
  return (color.r == 0 && color.g == 0 && color.b == 0);
}

// Check bounding box against mask
bool check_map_collision(SDL_Surface *mask, SDL_Rect rect) {
  if (!mask)
    return false;

  int startX = rect.x;
  int endX = rect.x + rect.w;
  int startY = rect.y;
  int endY = rect.y + rect.h;

  // Check bottom (Ground) - most important
  for (int x = startX; x < endX; x += 5) {
    if (is_colliding(mask, x, endY))
      return true;
  }
  // Check top (Head)
  for (int x = startX; x < endX; x += 5) {
    if (is_colliding(mask, x, startY))
      return true;
  }
  // Check Left
  for (int y = startY; y < endY; y += 5) {
    if (is_colliding(mask, startX, y))
      return true;
  }
  // Check Right
  for (int y = startY; y < endY; y += 5) {
    if (is_colliding(mask, endX, y))
      return true;
  }

  return false;
}

// ---------------------------------------------------------
// INITIALIZATION
// ---------------------------------------------------------

bool init_level1(GameContext *game, Player *p, Enemy *e, Background *bg) {
  // --- Init Background ---
  bg->texture = load_texture(game, "resources/image/niveau1.png");
  bg->mask = IMG_Load("resources/image/map1_masked.png");

  // Make sure mask exists. If not, maybe fallback to texture?
  // But texture is on GPU (SDL_Texture), we need CPU surface.
  // We can load niveau1.png as surface if map1_masked is missing.

  if (!bg->texture || !bg->mask) {
    printf("Failed to load level1 assets! Texture: %p, Mask: %p\n", bg->texture,
           bg->mask);
    return false;
  }

  SDL_QueryTexture(bg->texture, NULL, NULL, &bg->width, &bg->height);
  bg->camera = (SDL_Rect){0, 0, 1366, 768};

  // --- Init Player ---
  load_anim(game, p->anim_right, "resources/image/RW%d.png", 4);
  load_anim(game, p->anim_left, "resources/image/LW%d.png", 4);
  load_anim(game, p->anim_right_attack, "resources/image/RW%d.png", 4);
  load_anim(game, p->anim_left_attack, "resources/image/LW%d.png", 4);

  p->hearts[0] = load_texture(game, "resources/image/v4.png");
  p->hearts[1] = load_texture(game, "resources/image/v3.png");
  p->hearts[2] = load_texture(game, "resources/image/v2.png");
  p->hearts[3] = load_texture(game, "resources/image/v1.png");
  p->hearts[4] = load_texture(game, "resources/image/coeur.jpg");

  p->pos = (SDL_Rect){100, 400, 0, 0};

  if (p->anim_right[0])
    SDL_QueryTexture(p->anim_right[0], NULL, NULL, &p->pos.w, &p->pos.h);

  // Hitbox adjustments (visual sprite might have padding)
  p->pos.w -= 20;
  p->pos.h -= 10;

  p->dx = 0;
  p->dy = 0;
  p->speed = 6.0f;
  p->direction = 0;
  p->frame = 0;
  p->lives = 4;
  p->score = 0;
  p->on_ground = false;

  // --- Init Enemy ---
  load_anim(game, e->anim_right, "resources/image/ER%d.png", 4);
  load_anim(game, e->anim_left, "resources/image/EL%d.png", 4);

  e->pos = (SDL_Rect){800, 500, 0, 0};
  if (e->anim_left[0])
    SDL_QueryTexture(e->anim_left[0], NULL, NULL, &e->pos.w, &e->pos.h);

  e->direction = 1;
  e->speed = 3.0f;
  e->frame = 0;
  e->state = 0;

  return true;
}

void clean_level1(Player *p, Enemy *e, Background *bg) {
  if (bg->texture)
    SDL_DestroyTexture(bg->texture);
  if (bg->mask)
    SDL_FreeSurface(bg->mask);

  for (int i = 0; i < 4; i++) {
    if (p->anim_right[i])
      SDL_DestroyTexture(p->anim_right[i]);
    if (p->anim_left[i])
      SDL_DestroyTexture(p->anim_left[i]);
    if (e->anim_right[i])
      SDL_DestroyTexture(e->anim_right[i]);
    if (e->anim_left[i])
      SDL_DestroyTexture(e->anim_left[i]);
  }
}

// ---------------------------------------------------------
// UPDATE LOGIC
// ---------------------------------------------------------

void update_player(Player *p, Background *bg, const Uint8 *keys) {
  p->dx = 0;

  if (keys[SDL_SCANCODE_RIGHT]) {
    p->dx = p->speed;
    p->direction = 0;
  }
  if (keys[SDL_SCANCODE_LEFT]) {
    p->dx = -p->speed;
    p->direction = 1;
  }

  // --- X MOVEMENT ---
  int old_x = p->pos.x;
  p->pos.x += (int)p->dx;

  if (check_map_collision(bg->mask, p->pos)) {
    p->pos.x = old_x; // Collision! Revert.
    p->dx = 0;
  }

  // Clamp to world
  if (p->pos.x < 0)
    p->pos.x = 0;
  if (p->pos.x > bg->width - p->pos.w)
    p->pos.x = bg->width - p->pos.w;

  // --- Y MOVEMENT ---
  if (keys[SDL_SCANCODE_SPACE] && p->on_ground) {
    p->dy = JUMP_FORCE;
    p->on_ground = false;
    p->is_jumping = true;
  }

  p->dy += GRAVITY;
  if (p->dy > MAX_FALL_SPEED)
    p->dy = MAX_FALL_SPEED;

  int old_y = p->pos.y;
  p->pos.y += (int)p->dy;
  p->on_ground = false;

  if (check_map_collision(bg->mask, p->pos)) {
    if (p->dy > 0) { // Landing
      p->on_ground = true;
      // Snap to top of block?
      // For now, just revert to old_y
      // A better way is to move down pixel by pixel until hit?
      // Or binary search. Simple revert is "sticky" but works basic.

      // Revert completely to avoid getting stuck
      p->pos.y = old_y;

      // Try to move down 1 pixel at a time to get closer to ground
      while (!check_map_collision(
          bg->mask, (SDL_Rect){p->pos.x, p->pos.y + 1, p->pos.w, p->pos.h})) {
        p->pos.y++;
      }
    } else { // Bonk head
      p->pos.y = old_y;
    }
    p->dy = 0;
  }

  // Debug: Print position if falling deep
  // if(p->pos.y > 600) printf("Player low: %d\n", p->pos.y);

  // --- ANIMATION ---
  if (p->dx != 0) {
    p->anim_timer++;
    if (p->anim_timer > 5) {
      p->frame = (p->frame + 1) % 4;
      p->anim_timer = 0;
    }
  } else {
    p->frame = 0;
  }

  // --- SCROLLING ---
  int center_x = 1366 / 2;
  bg->camera.x = p->pos.x - center_x;

  if (bg->camera.x < 0)
    bg->camera.x = 0;
  if (bg->camera.x > bg->width - bg->camera.w)
    bg->camera.x = bg->width - bg->camera.w;
}

void update_enemy(Enemy *e, Player *p) {
  e->anim_timer++;
  if (e->anim_timer > 10) {
    e->frame = (e->frame + 1) % 4;
    e->anim_timer = 0;
  }
}

// ---------------------------------------------------------
// RENDER
// ---------------------------------------------------------

void render_level1(GameContext *game, Player *p, Enemy *e, Background *bg) {
  SDL_RenderClear(game->renderer);

  SDL_RenderCopy(game->renderer, bg->texture, &bg->camera, NULL);

  SDL_Rect rel_p = p->pos;
  rel_p.x -= bg->camera.x;
  rel_p.y -= bg->camera.y;

  SDL_Rect rel_e = e->pos;
  rel_e.x -= bg->camera.x;
  rel_e.y -= bg->camera.y;

  SDL_Texture *current_tex = NULL;
  if (p->direction == 0)
    current_tex = p->anim_right[p->frame];
  else
    current_tex = p->anim_left[p->frame];

  if (current_tex) {
    SDL_RenderCopy(game->renderer, current_tex, NULL, &rel_p);
  }

  SDL_Texture *e_tex = NULL;
  if (e->direction == 0)
    e_tex = e->anim_right[e->frame];
  else
    e_tex = e->anim_left[e->frame];

  if (e_tex) {
    SDL_RenderCopy(game->renderer, e_tex, NULL, &rel_e);
  }

  // Draw HUD Hearts
  for (int i = 0; i < p->lives; i++) {
    SDL_Rect heart_pos = {10 + (35 * i), 10, 30, 30};
    if (p->hearts[0])
      SDL_RenderCopy(game->renderer, p->hearts[0], NULL, &heart_pos);
  }

  SDL_RenderPresent(game->renderer);
}

// ---------------------------------------------------------
// REFACTORED MAIN LOOP FOR LEVEL 1
// ---------------------------------------------------------

void play_level1(GameContext *game) {
  Player p;
  Enemy e;
  Background bg;

  if (!init_level1(game, &p, &e, &bg)) {
    printf("Failed to init level 1\n");
    return;
  }

  bool running = true;
  SDL_Event event;
  const Uint8 *keys = SDL_GetKeyboardState(NULL);

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

    update_player(&p, &bg, keys);
    update_enemy(&e, &p);
    render_level1(game, &p, &e, &bg);

    SDL_Delay(16);
  }

  clean_level1(&p, &e, &bg);
}
