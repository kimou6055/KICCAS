#include "fonctions.h"
#include "game.h"
#include "intro.h"
#include "levels.h"
#include "option.h"
#include "puissance4.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  // 1. Initialize access to GameContext
  GameContext game;
  if (!init_game(&game)) {
    printf("Failed to initialize game!\n");
    return 1;
  }

  // 2. Intro Sequence
  intro(&game);

  // 3. Main Loop (State Machine)
  // States: 0=Menu, 1=Play, 2=Options, 3=Credits (Not Impl)
  int current_state = 0;

  while (game.running) {
    switch (current_state) {
    case 0: // Menu
    {
      int choice = afficher_menu(&game);
      if (choice == 0)
        game.running = false; // Quit
      else if (choice == 1)
        current_state = 1; // Play
      else if (choice == 2)
        current_state = 2; // Options
      else if (choice == 3)
        current_state = 3; // Credits (placeholder)
    } break;

    case 1: // Game Loop (Levels 1-4)
    {
      int next_level = 1; // Start at Level 1
      while (next_level > 0 && next_level <= 4 && game.running) {
        next_level = play_level(&game, next_level);
      }
      current_state = 0; // Back to menu when done or dead
    } break;

    case 2: // Options
    {
      int ret = afficher_option(&game);
      if (ret == 0)
        game.running = false; // Quit from options
      else
        current_state = 0; // Back to menu
    } break;

    case 3: // Credits (Placeholder - just goes back to menu)
      SDL_Delay(500);
      current_state = 0;
      break;

    default:
      game.running = false;
      break;
    }
  }

  // 4. Cleanup
  close_game(&game);

  return 0;
}
