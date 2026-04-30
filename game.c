#include "game.h"
#include <stdlib.h>

//definizione figure (tutte da 5 quadtratini)
const Piece PIECES[NUM_PIECES] = {
    {{{0,0}, {1,0}, {2,0}, {3,0}, {4,0}}, 1}, // lettera I 
    {{{0,0}, {0,1}, {1,1}, {2,1}, {2,0}}, 2}, // lettera U
    {{{0,0}, {1,0}, {2,0}, {1,1}, {1,2}}, 3}, // lettera T
    {{{0,0}, {0,1}, {1,1}, {2,1}, {2,2}}, 4}, // lettera Z
    {{{1,0}, {0,1}, {0,2}, {1,1}, {2,1}}, 5}, // lettera F
    {{{0,0}, {1,0}, {2,0}, {3,0}, {3,1}}, 6}, // lettera L
    {{{1,0}, {2,0}, {3,0}, {0,1}, {1,1}}, 7}, // lettera N
    {{{1,0}, {2,0}, {0,1}, {1,1}, {2,1}}, 8}, // lettera P
    {{{0,0}, {1,0}, {2,0}, {2,1}, {2,2}}, 9}, // lettera V
    {{{0,0}, {1,0}, {1,1}, {2,1}, {2,2}}, 10}, // lettera W
    {{{1,0}, {0,1}, {1,1}, {2,1}, {1,2}}, 11}, // lettera X
    {{{1,0}, {0,1}, {1,1}, {2,1}, {3,1}}, 12}, // lettera Y
};

//funzione che inizializza e resetta il campo
void init_game(game_state *game){
    for (int i = 0; i < ROWS; i++){
        for (int j = 0; j < COLS; j++){
            game->board[i][j] = 0;
        }
    }
    game->score = 0;
    spawn_piece(game);

    //inizializzazione primo pezzo
    //game->active_piece = PIECES[0];
    //game->active_x = 3;
    //game->active_y = 0;
}

//questa funzione serve a far comparire un pezzo e farlo comparire in alto e al centro
void spawn_piece(game_state* game){
    int piece_index = rand() % NUM_PIECES; //sceglie numero casuale da 0 a 11

    game->active_piece = PIECES[piece_index];
    game->active_x = COLS/2 - 2;
    game->active_y = 0;
}