#ifndef LEVEL1_H
#define LEVEL1_H

#include "game.h"

// Define states for Entity/Player
typedef enum { IDLE, WALKING, JUMPING, ATTACKING, DEAD } EntityState;

// Background / Scrolling Structure
typedef struct {
  SDL_Texture *texture;
  SDL_Surface *mask; // Collision mask
  SDL_Rect camera;   // The part of the image we see
  int width, height; // Texture dimensions
} Background;

// Player Structure (Ported from Personne)
typedef struct {
  SDL_Texture *anim_right[4];
  SDL_Texture *anim_left[4];
  SDL_Texture *anim_right_attack[4]; // Optional
  SDL_Texture *anim_left_attack[4];  // Optional
  SDL_Texture *hearts[5];            // Heart images

  SDL_Rect pos; // Position in the world

  float dx, dy; // Velocity
  float speed;
  int direction; // 0: Right, 1: Left
  int frame;     // Animation frame
  int anim_timer;

  int lives;
  int score;
  bool on_ground;
  bool is_jumping;
} Player;

// Enemy Structure (Ported from Ennemi)
typedef struct {
  SDL_Texture *anim_right[4];
  SDL_Texture *anim_left[4];
  SDL_Texture *anim_right_attack[4];
  SDL_Texture *anim_left_attack[4];

  SDL_Rect pos;
  float start_x; // Patrol start
  float end_x;   // Patrol end
  int direction; // 0: Right, 1: Left
  float speed;
  int frame;
  int anim_timer;

  // Simple state machine
  int state; // 0: WAITING, 1: FOLLOWING, 2: ATTACKING
} Enemy;

// Main entry point for Level 1
void play_level1(GameContext *game);

#endif
