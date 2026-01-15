#include "puissance4.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NB_COLONNES 7
#define NB_LIGNES 6
#define VIDE ' '
#define ROUGE 'x'
#define JAUNE 'o'

// Define sizes for drawing
#define P4_CELL_SIZE 80
#define P4_MARGIN 10
#define P4_OFFSET_X 300
#define P4_OFFSET_Y 100

typedef struct {
  char grid[NB_COLONNES][NB_LIGNES];
  int currentPlayer; // 0 for Yellow (Player/AI), 1 for Red (Player) -- Original
                     // code had logic inverted? Logic: 0=Yellow, 1=Red
  // Original: joueur==FALSE -> JAUNE, joueur==TRUE -> ROUGE
  int moveCount;
  bool gameOver;
  bool isHumanTurn;
  bool againstAI; // Set to true for single player
} P4State;

void init_p4(P4State *state) {
  for (int x = 0; x < NB_COLONNES; x++) {
    for (int y = 0; y < NB_LIGNES; y++) {
      state->grid[x][y] = VIDE;
    }
  }
  state->currentPlayer = 1; // Red starts
  state->moveCount = 0;
  state->gameOver = false;
  state->isHumanTurn = true;
  state->againstAI = true;
}

bool check_win(char grid[NB_COLONNES][NB_LIGNES]) {
  // Horizontal
  for (int y = 0; y < NB_LIGNES; y++) {
    for (int x = 0; x <= NB_COLONNES - 4; x++) {
      char c = grid[x][y];
      if (c != VIDE && c == grid[x + 1][y] && c == grid[x + 2][y] &&
          c == grid[x + 3][y])
        return true;
    }
  }
  // Vertical
  for (int x = 0; x < NB_COLONNES; x++) {
    for (int y = 0; y <= NB_LIGNES - 4; y++) {
      char c = grid[x][y];
      if (c != VIDE && c == grid[x][y + 1] && c == grid[x][y + 2] &&
          c == grid[x][y + 3])
        return true;
    }
  }
  // Diagonal Down-Right
  for (int x = 0; x <= NB_COLONNES - 4; x++) {
    for (int y = 0; y <= NB_LIGNES - 4; y++) {
      char c = grid[x][y];
      if (c != VIDE && c == grid[x + 1][y + 1] && c == grid[x + 2][y + 2] &&
          c == grid[x + 3][y + 3])
        return true;
    }
  }
  // Diagonal Up-Right
  for (int x = 0; x <= NB_COLONNES - 4; x++) {
    for (int y = 3; y < NB_LIGNES; y++) {
      char c = grid[x][y];
      if (c != VIDE && c == grid[x + 1][y - 1] && c == grid[x + 2][y - 2] &&
          c == grid[x + 3][y - 3])
        return true;
    }
  }

  return false;
}

int get_valid_row(char grid[NB_COLONNES][NB_LIGNES], int col) {
  for (int y = NB_LIGNES - 1; y >= 0; y--) {
    if (grid[col][y] == VIDE)
      return y;
  }
  return -1;
}

int ai_move(char grid[NB_COLONNES][NB_LIGNES]) {
  // Simple random AI for now, as in original
  int col;
  do {
    col = rand() % NB_COLONNES;
  } while (get_valid_row(grid, col) == -1);
  return col;
}

void draw_p4(GameContext *game, P4State *state) {
  SDL_SetRenderDrawColor(game->renderer, 0, 0, 50, 255); // Dark blue background
  SDL_RenderClear(game->renderer);

  // Draw Board
  SDL_Rect cellRect;
  cellRect.w = P4_CELL_SIZE;
  cellRect.h = P4_CELL_SIZE;

  for (int x = 0; x < NB_COLONNES; x++) {
    for (int y = 0; y < NB_LIGNES; y++) {
      cellRect.x = P4_OFFSET_X + x * (P4_CELL_SIZE + P4_MARGIN);
      cellRect.y = P4_OFFSET_Y + y * (P4_CELL_SIZE + P4_MARGIN);

      // Draw slot background (Blue)
      SDL_SetRenderDrawColor(game->renderer, 0, 0, 150, 255);
      SDL_RenderFillRect(game->renderer, &cellRect);

      // Draw Piece
      char c = state->grid[x][y];
      if (c == ROUGE) {
        SDL_SetRenderDrawColor(game->renderer, 255, 0, 0, 255);
        SDL_Rect pieceRect = {cellRect.x + 5, cellRect.y + 5, P4_CELL_SIZE - 10,
                              P4_CELL_SIZE - 10};
        SDL_RenderFillRect(game->renderer, &pieceRect);
      } else if (c == JAUNE) {
        SDL_SetRenderDrawColor(game->renderer, 255, 255, 0, 255);
        SDL_Rect pieceRect = {cellRect.x + 5, cellRect.y + 5, P4_CELL_SIZE - 10,
                              P4_CELL_SIZE - 10};
        SDL_RenderFillRect(game->renderer, &pieceRect);
      } else {
        SDL_SetRenderDrawColor(game->renderer, 255, 255, 255,
                               255); // White empty slot hole
        SDL_Rect pieceRect = {cellRect.x + 5, cellRect.y + 5, P4_CELL_SIZE - 10,
                              P4_CELL_SIZE - 10};
        SDL_RenderFillRect(game->renderer, &pieceRect);
      }
    }
  }

  // Draw Status Text
  if (game->font) {
    char msg[64];
    SDL_Color white = {255, 255, 255, 255};
    if (state->gameOver) {
      sprintf(msg, "Game Over! %s Wins! (Press ESC)",
              (state->currentPlayer == 1 ? "Yellow" : "Red"));
    } else {
      sprintf(msg, "Turn: %s", (state->currentPlayer == 1 ? "Red" : "Yellow"));
    }

    SDL_Surface *tsurf = TTF_RenderText_Solid(game->font, msg, white);
    if (tsurf) {
      SDL_Texture *ttex = SDL_CreateTextureFromSurface(game->renderer, tsurf);
      SDL_Rect tpos = {P4_OFFSET_X, 20, tsurf->w, tsurf->h};
      SDL_RenderCopy(game->renderer, ttex, NULL, &tpos);
      SDL_DestroyTexture(ttex);
      SDL_FreeSurface(tsurf);
    }
  }

  SDL_RenderPresent(game->renderer);
}

void play_puissance4(GameContext *game) {
  P4State state;
  init_p4(&state);
  srand(time(NULL));

  SDL_Event event;
  bool quit = false;

  while (!quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = true;
      } else if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          quit = true;
        }
      } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (!state.gameOver && state.isHumanTurn) {
          int mx = event.button.x;
          int my = event.button.y;

          // Simple hit detection for columns
          if (mx >= P4_OFFSET_X &&
              mx <= P4_OFFSET_X + NB_COLONNES * (P4_CELL_SIZE + P4_MARGIN)) {
            int col = (mx - P4_OFFSET_X) / (P4_CELL_SIZE + P4_MARGIN);
            if (col >= 0 && col < NB_COLONNES) {
              int row = get_valid_row(state.grid, col);
              if (row != -1) {
                state.grid[col][row] =
                    (state.currentPlayer == 1) ? ROUGE : JAUNE;
                state.moveCount++;
                if (check_win(state.grid)) {
                  state.gameOver = true;
                } else if (state.moveCount >= NB_COLONNES * NB_LIGNES) {
                  state.gameOver = true; // Draw
                } else {
                  // Switch turn
                  state.currentPlayer = (state.currentPlayer == 1) ? 0 : 1;
                  state.isHumanTurn =
                      !state.againstAI; // If AI, next is not human
                }
              }
            }
          }
        } else if (state.gameOver) {
          quit = true; // Click to exit on game over
        }
      }
    }

    // AI Turn
    if (!state.gameOver && !state.isHumanTurn && state.againstAI) {
      SDL_Delay(500); // Thinking delay
      int col = ai_move(state.grid);
      int row = get_valid_row(state.grid, col);
      if (row != -1) {
        state.grid[col][row] = (state.currentPlayer == 1) ? ROUGE : JAUNE;
        state.moveCount++;
        if (check_win(state.grid)) {
          state.gameOver = true;
        } else if (state.moveCount >= NB_COLONNES * NB_LIGNES) {
          state.gameOver = true;
        } else {
          state.currentPlayer = (state.currentPlayer == 1) ? 0 : 1;
          state.isHumanTurn = true;
        }
      }
    }

    draw_p4(game, &state);
    SDL_Delay(16); // ~60 FPS cap
  }
}
