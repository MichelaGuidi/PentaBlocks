#include "game.h"
#include <stdlib.h>
#include <stdbool.h>

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

//funzione booleana serve per capire se un pezzo è fuori dal campo o se entra in collisione con blocchi fissi
bool can_place(const game_state* game, Piece piece, int x, int y){
    //effettua il controllo per ogni quadratino del pezzo
    for (int i = 0; i < BLOCKS_PER_PIECE; i++){
        int nx = x + piece.shape[i].dx;
        int ny = y + piece.shape[i].dy;

        if (nx < 0 || nx >= COLS || ny < 0 || ny >= ROWS){
            return false;
        }

        if (game->board[ny][nx] != 0){
            return false;
        }
    }
    return true;
}

//questa funzione restituisce un nuovo pezzo dello stesso tipo di active_piece, ma ruotato di 90 gradi
Piece rotate_piece(Piece piece){
    Piece rotated;
    rotated.color_id = piece.color_id;

    //ruota un punto (x,y) di 90 gradi e quindi in (-y,x). Ripete questa cosa per gli offset di ogni quadratino
    for (int i = 0; i < BLOCKS_PER_PIECE; i++){
        rotated.shape[i].dx = -piece.shape[i].dy;
        rotated.shape[i].dy = piece.shape[i].dx;
    }

    //dobbiamo controllare che non ci siano offset negativi, quindi cerchiamo i più piccoli
    int min_dx = rotated.shape[0].dx;
    int min_dy = rotated.shape[0].dy;

    for (int i = 0; i < BLOCKS_PER_PIECE; i++){
        if (rotated.shape[i].dx < min_dx) min_dx = rotated.shape[i].dx;
        if (rotated.shape[i].dy < min_dy) min_dy = rotated.shape[i].dy;
    }

    //la figura viene traslata verso destra proprio per evitare offset negativi
    for (int i = 0; i < BLOCKS_PER_PIECE; i++){
        rotated.shape[i].dx -= min_dx;
        rotated.shape[i].dy -= min_dy;
    }

    return rotated;
}

//funzione che blocca un pezzo
//per ogni quadratino, cambia il colore della board
void lock_piece(game_state *game){
    for (int i = 0; i < BLOCKS_PER_PIECE; i++){
        int nx = game->active_x + game->active_piece.shape[i].dx;
        int ny = game->active_y + game->active_piece.shape[i].dy;

        if (nx >= 0 && nx < COLS && ny >= 0 && ny < ROWS){
            game->board[ny][nx] = game->active_piece.color_id; //il valore associato a board[ny][nx] diventa diverso da zero
        }
    }
}