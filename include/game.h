#ifndef GAME_H
#define GAME_H
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

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
    uint8_t color_id;
} Piece;

//byte_field
typedef struct{
    uint8_t board[ROWS][COLS]; //cambio da int a uint8_t perché più adatto per byte_fild
    int score; //punteggio partita
    int level; //livello attuale
    int lines_cleared; //numero di righe completate
    int next_level_lines; //soglia di righe per passare al prossimo livello
    int next_level_score; //soglia punteggio per il prossimo livello

    bool level_complete; //livello superato o meno
    int level_target_score; //punteggio raggiunto, da mostrare nel popup

    bool game_over_screen; //true se partita persa

    int flash_timer_ms; //campo per controllare il flash quando una riga è piena

    Piece active_piece; //definiamo il pezzo che deve muoversi
    int active_x; // colonna in cui si trova (l'inizio)
    int active_y; // riga in cui si trova (l'inizio)

    Piece next_piece; //prossimo pezzo che apparirà nel gioco
} game_state;

extern const Piece PIECES[NUM_PIECES];

void init_game(game_state *game);
void spawn_piece(game_state *game);
bool can_place(const game_state* game, Piece piece, int x, int y);
Piece rotate_piece(Piece piece);
void lock_piece(game_state *game);

bool top_row_occupied(game_state *game);

void game_over(game_state *game);

void clear_lines(game_state* game);

void update_level(game_state *game);
int get_drop_interval_ms(const game_state* game);
void start_next_level(game_state* game);
#endif