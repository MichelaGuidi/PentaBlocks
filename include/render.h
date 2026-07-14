#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include "game.h"

//componenti del menu
typedef enum {
    MENU_RESUME,
    MENU_RESTART,
    MENU_QUIT,
    MENU_ITEMS_COUNT
} PauseMenuItem;

void draw_hud_box(SDL_Renderer* renderer, SDL_Rect rect);
void draw_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color);
void draw_score(SDL_Renderer* renderer, TTF_Font* font1, TTF_Font* font2, game_state* game);
void draw_board_frame(SDL_Renderer* renderer);
void draw_pause_button(SDL_Renderer* renderer, TTF_Font* font1, SDL_Rect rect, bool clicked);
void draw_sidebar(SDL_Renderer* renderer);
void draw_board(SDL_Renderer* renderer, game_state* game);
void draw_active_piece(SDL_Renderer* renderer, game_state* game);
void draw_next_panel(SDL_Renderer* renderer, TTF_Font* font1);
void draw_next_piece(SDL_Renderer* renderer, game_state* game);
void draw_pause_menu(SDL_Renderer* renderer, TTF_Font* font1, TTF_Font* font2, int selected, int hovered);
void draw_game_over(SDL_Renderer* renderer, TTF_Font* font1, TTF_Font* font2, int selected, int hovered);
void draw_level_complete_box(SDL_Renderer* renderer, TTF_Font* font1, TTF_Font* font2, game_state* game, int selected, int hovered);
void draw_line_clear_flash(SDL_Renderer* renderer, game_state* game);
void draw_ghost_piece(SDL_Renderer* renderer, game_state* game);
void draw_first_box(SDL_Renderer* renderer, TTF_Font* font1, TTF_Font* font2, int selected, int hovered);
void draw_controls_box(SDL_Renderer* renderer, TTF_Font* font1, TTF_Font* font2);
void draw_min_level_score(SDL_Renderer* renderer, TTF_Font* font1, game_state* game);

#endif