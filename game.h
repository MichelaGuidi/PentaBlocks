#ifndef GAME_H
#define GAME_H
#include <stdint.h>

#define ROWS 20
#define COLS 10
#define BLOCKS_PER_PIECE 5
#define NUM_PIECES 12

//struttura singolo quadrato
typedef struct{
    int dx;
    int dy;
} Offset;

//struttura di un pezzo
typedef struct {
    Offset shape[5]; //cambiato da 4 a 5 per realizzare dei pentamini al posto dei tetramini
    int color_id;
} Piece;

//byte_field
typedef struct{
    uint8_t board[ROWS][COLS]; //cambio da int a uint8_t perché più adatto per byte_fild
    int score; //punteggio partita

    Piece active_piece; //definiamo il pezzo che deve muoversi
    int active_x; // colonna in cui si trova (l'inizio)
    int active_y; // riga in cui si trova (l'inizio)
} game_state;

extern const Piece PIECES[NUM_PIECES];

void init_game(game_state *game);
void spawn_piece(game_state *game);

#endif