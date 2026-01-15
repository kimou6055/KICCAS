#ifndef LEVELS_H
#define LEVELS_H

#include "game.h"

// --- PHYSICS CONSTANTS ---
#define GRAVITY 0.55f
#define JUMP_FORCE -17.5f
#define MAX_FALL_SPEED 12.0f
#define ACCELERATION 0.4f
#define FRICTION 0.3f
#define MAX_SPEED 7.0f

// --- STRUCTURES ---

typedef struct {
  SDL_Texture *texture;
  SDL_Surface *mask;
  SDL_Rect camera;
  int width, height;
  float scale_x, scale_y;
} LevelMap;

typedef struct {
  SDL_Texture *anim_right[4];
  SDL_Texture *anim_left[4];
  SDL_Texture *hearts[5];

  // Physics State
  float x, y;   // Precise float position
  float vx, vy; // Velocity

  SDL_Rect rect; // Collision Box (visuals might be offset)

  int direction; // 0: Right, 1: Left
  int frame;
  int anim_timer;

  int lives;
  int score;

  // Flags
  bool on_ground;
  bool is_jumping;
} Player;

typedef enum { ENEMY_PATROL, ENEMY_CHASE } EnemyType;

typedef struct {
  SDL_Texture *anim_right[4];
  SDL_Texture *anim_left[4];

  float x, y;
  float vx, vy;
  SDL_Rect rect;

  int direction;
  int frame;
  int anim_timer;
  bool active; // If false, enemy is dead

  EnemyType type;
  float start_x, end_x; // Patrol checks
} Enemy;

#define MAX_ENEMIES 20

// --- PROTOTYPES ---

// Call this to start the game loop for a specific level (1-4)
// Returns the next level to load (e.g., 2), or 0 for Menu, -1 for Exit
int play_level(GameContext *game, int level_id);

#endif
