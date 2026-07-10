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

//funzione che seglie una figura casuale. Prima rand era solo in spawn_piece
//ma adesso serve anche per creare il 'next_piece'
static Piece random_piece(void){
    int piece_index = rand() % NUM_PIECES;
    return PIECES[piece_index];
}

//funzione che inizializza e resetta il campo
void init_game(game_state *game){
    for (int i = 0; i < ROWS; i++){
        for (int j = 0; j < COLS; j++){
            game->board[i][j] = 0;
        }
    }
    game->score = 0;
    game->level = 1;
    game->lines_cleared = 0;
    game->next_level_lines = 2;
    game->next_level_score = 200;
    game->level_complete = false;

    game->game_over_screen = false;

    //appena la partita comincia abbiamo un pezzo casuale nella griglia e uno a lato
    game->active_piece = random_piece();
    game->next_piece = random_piece();
    game->active_x = COLS/2-2; //posizione iniziale orizzontale
    game->active_y = 0; //posizione iniziale verticale
    //inizializzazione primo pezzo
}

//questa funzione serve a far comparire un pezzo e farlo comparire in alto e al centro
void spawn_piece(game_state* game){
    game->active_piece = game->next_piece; //prossimo pezzo in griglia è quello mostrato a lato
    game->next_piece = random_piece();
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

//la funzione controlla se c'è almeno un quadratino della prima riga che ha un pezzo fisso
bool top_row_occupied(game_state* game){
    for (int i = 0; i < COLS; i++){
        if (game->board[0][i] != 0){
            return true;
        }
    } 
    return false;
}

//la funzione pulisce le righe piene e fa scendere le righe più in alto
void clear_lines(game_state* game){
    int c = 0;
    //a partire dall'ultima riga controlla se ce n'è almeno una completamente piena
    for (int i = ROWS - 1; i >= 0; i--){
        bool full = true; //parte da piena

        //se trova un quadratino vuoto, passa alla prossima riga
        for (int j = 0; j < COLS; j++){
            if (game->board[i][j] == 0){
                full = false;
                break;
            }
        }

        //se trova la riga piena
        if (full){
            c++;
            //sostituisce ogni riga con la riga sopra, cominciando da quella cancellata
            for (int y = i; y > 0; y--){
                for (int x = 0; x < COLS; x++){
                    game->board[y][x] = game->board[y-1][x];
                }
            }

            //la prima riga sicuramente è vuota
            for (int x = 0; x < COLS; x++){
                game->board[0][x] = 0;
            }

            i++; //incremento per controllora di nuovo la riga attuale, dato che nel for viene fatto i--
        }
    }
    //aumenta il punteggio, ancora non utilizzato
    game->score += c*100;
    game->lines_cleared += c;
    update_level(game);
    return;
}

//questa funzione aggiorna il livello e aumenta la soglia di righe complete e punteggio
//per passare al prossimo livello
void update_level(game_state* game){
    while(game->lines_cleared >= game->next_level_lines || game->score >= game->next_level_score){
        game->level++;
        game->next_level_lines +=2;
        game->next_level_score += 200;
        game->level_complete = true;
    }
}

//cominciando una nuova partita, il livello resta quello aumentato, come anche le soglie
//mentre il resto viene riportato allo stato iniziale
void start_next_level(game_state* game){
    for (int i = 0; i < ROWS; i++){
        for (int j = 0; j < COLS; j++){
            game->board[i][j] = 0;
        }
    }

    game->score = 0;
    game->lines_cleared = 0;
    game->level_complete = false;
    spawn_piece(game);
}