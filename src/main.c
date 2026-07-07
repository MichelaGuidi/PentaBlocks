#include "game.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

//Dimensioni finestra
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 640;

#define T_SIZE 30 //numero di pixel che occupa un blocco
#define DROP_INTERVAL 1000 //intervallo di 1000 ms (ogni quanto il pezzo scende verso il basso)
#define FRAME_DELAY 16 //limite per frame impostato a 16 ms 

#define BOARD_OFFSET_X 50
#define BOARD_OFFSET_Y 20

//funzione che scrive una stringa a schermo
void draw_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color){
    //crea una surface contenente il testo renderizzato con il font scelto
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
    if (surface == NULL){
        printf("Errore rendering testo: %s\n", TTF_GetError());
        return;
    }

    //converte la surface in una texture, che SDL può disegnare nel renderer
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL){
        printf("Errore crazione texture testo: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    
    //definisce il rettangolo di destinazione: posizione x, y e dimensioni del testo
    SDL_Rect dest = {x, y, surface->w, surface->h};

    //chiusura della surface
    SDL_FreeSurface(surface);
    //disegna la texture nel renderer
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    //libera la texture dopo il disegno
    SDL_DestroyTexture(texture);
}

void draw_score(SDL_Renderer* renderer, TTF_Font* font, game_state* game){
    char score_text[64];
    //costruisce una stringa contenente il punteggio corrente
    snprintf(score_text, sizeof(score_text), "Score: %d", game->score);

    SDL_Color white = {255, 255, 255, 255};
    //disegna il testo nella zona laterale destra della finestra
    draw_text(renderer, font, score_text, COLS*T_SIZE + 120, 30, white);
}

//funzione che disegna bordo della griglia
void draw_board_frame(SDL_Renderer* renderer){
    SDL_Rect frame = {BOARD_OFFSET_X - 2, BOARD_OFFSET_Y - 2, COLS*T_SIZE + 4, ROWS* T_SIZE + 4};

    //bordo esterno luminoso
    SDL_SetRenderDrawColor(renderer, 80, 140, 255, 255);
    SDL_RenderDrawRect(renderer, &frame);

    //secondo bordo interno per dare più spessore
    //SDL_Rect inner =  {BOARD_OFFSET_X - 5, BOARD_OFFSET_Y - 5, COLS* T_SIZE + 6, ROWS*T_SIZE+6};
    //SDL_SetRenderDrawColor(renderer, 40, 80, 180, 255);
    //SDL_RenderDrawRect(renderer, &inner);
}

void draw_sidebar(SDL_Renderer* renderer){
    SDL_Rect panel = {COLS* T_SIZE + 100, 18, SCREEN_WIDTH - (COLS*T_SIZE + 200), SCREEN_HEIGHT - 200};

    //riempimento pannello
    SDL_SetRenderDrawColor(renderer, 12, 18, 45, 255);
    SDL_RenderFillRect(renderer, &panel);

    //bordo del pannello
    SDL_SetRenderDrawColor(renderer, 90, 120, 220, 255);
    SDL_RenderDrawRect(renderer, &panel);
}

//nuova funzione per colorare i pezzi
void get_piece_rgb(uint8_t color_id, Uint8* r, Uint8* g, Uint8* b){
    switch (color_id){
        case 1:  *r =   0; *g = 255; *b = 255; break; //ciano
        case 2:  *r = 255; *g = 255; *b =   0; break; //giallo
        case 3:  *r = 160; *g =  32; *b = 240; break; //viola
        case 4:  *r =   0; *g = 255; *b =   0; break; //verde
        case 5:  *r = 255; *g = 105; *b = 180; break; //rosa
        case 6:  *r = 255; *g = 165; *b =   0; break; //arancione
        case 7:  *r =   0; *g = 128; *b = 255; break; //blu
        case 8:  *r = 255; *g =   0; *b =   0; break; //rosso
        case 9:  *r =   0; *g = 200; *b = 120; break; //verde acqua
        case 10: *r = 200; *g = 200; *b = 255; break; //lilla chiaro
        case 11: *r = 255; *g = 255; *b = 255; break; //bianco
        case 12: *r = 180; *g = 120; *b =   0; break; //ocra
        default: *r = 180; *g = 180; *b = 180; break; //fallback
    }
}

//varianti più chiare e più scure dei colori per fare i bordi 3D
static Uint8 lighter(Uint8 c) { return (c > 215) ? 255 : c + 40; }
static Uint8 darker(Uint8 c)  { return (c <  40) ?   0 : c - 40; }

//funzione per creare effetto 3D della cella piena
void draw_block_3d(SDL_Renderer* renderer, SDL_Rect rect, uint8_t color_id, int active) {
    Uint8 r, g, b;
    get_piece_rgb(color_id, &r, &g, &b);

    SDL_Rect inner = { rect.x + 2, rect.y + 2, rect.w - 4, rect.h - 4 };

    // base
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderFillRect(renderer, &inner);

    //luce in alto e a sinistra
    SDL_SetRenderDrawColor(renderer, lighter(r), lighter(g), lighter(b), 255);
    SDL_RenderDrawLine(renderer, rect.x, rect.y, rect.x + rect.w - 1, rect.y);
    SDL_RenderDrawLine(renderer, rect.x, rect.y, rect.x, rect.y + rect.h - 1);
    SDL_RenderDrawLine(renderer, rect.x + 1, rect.y + 1, rect.x + rect.w - 2, rect.y + 1);
    SDL_RenderDrawLine(renderer, rect.x + 1, rect.y + 1, rect.x + 1, rect.y + rect.h - 2);

    //ombra in basso e a destra
    SDL_SetRenderDrawColor(renderer, darker(r), darker(g), darker(b), 255);
    SDL_RenderDrawLine(renderer, rect.x, rect.y + rect.h - 1, rect.x + rect.w - 1, rect.y + rect.h - 1);
    SDL_RenderDrawLine(renderer, rect.x + rect.w - 1, rect.y, rect.x + rect.w - 1, rect.y + rect.h - 1);
    SDL_RenderDrawLine(renderer, rect.x + 1, rect.y + rect.h - 2, rect.x + rect.w - 2, rect.y + rect.h - 2);
    SDL_RenderDrawLine(renderer, rect.x + rect.w - 2, rect.y + 1, rect.x + rect.w - 2, rect.y + rect.h - 2);

    // bordo interno lucido
    SDL_SetRenderDrawColor(renderer, lighter(r), lighter(g), lighter(b), 120);
    SDL_RenderDrawRect(renderer, &inner);

    // evidenziazione pezzo attivo
    if (active) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 120);
        SDL_Rect glow = { rect.x - 1, rect.y - 1, rect.w + 2, rect.h + 2 };
        SDL_RenderDrawRect(renderer, &glow);
    }
}

//funzione che disegna la griglia con i pezzi fissi
void draw_board(SDL_Renderer* renderer, game_state* game){
    for (int i = 0; i < ROWS; i++){
        for (int j = 0; j < COLS; j++){
            SDL_Rect rect;
            rect.x = BOARD_OFFSET_X + j * T_SIZE; //il pixel in cui comincia il quadratino (in colonna)
            rect.y = BOARD_OFFSET_Y + i * T_SIZE; //il pixel in cui comincia il quadratino (in riga)
            rect.w = T_SIZE; //larghezza del quadratino
            rect.h = T_SIZE; //altezza del quadratino

            if (game->board[i][j] == 0){ //se la cella è vuota disegna solo il bordo grigio
                SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
                SDL_RenderDrawRect(renderer, &rect);
            } else { //se è piena disegna blocco dello stesso colore del pezzo
                draw_block_3d(renderer, rect, game->board[i][j], 0);
            }
        }
    }
}

//funzione che disegna un pezzo sulla griglia che dovrà muoversi
void draw_active_piece(SDL_Renderer* renderer, game_state* game){

    for (int i = 0; i < 5; i++){
        SDL_Rect rect;

        rect.x = BOARD_OFFSET_X + (game->active_x + game->active_piece.shape[i].dx) * T_SIZE;
        rect.y = BOARD_OFFSET_Y + (game->active_y + game->active_piece.shape[i].dy) * T_SIZE;
        rect.w = T_SIZE;
        rect.h = T_SIZE;

        draw_block_3d(renderer, rect, game->active_piece.color_id, 0);
        
    }
}

//funzione che disegna il next_piece a lato
void draw_next_piece(SDL_Renderer* renderer, game_state* game){
    int preview_x = COLS * T_SIZE + 120; //inizio dell'area di preview a lato
    int preview_y = 300; //altezza della preview nella finestra
    int preview_block = 18; //blocchi più piccoli rispetto alla griglia principale

    SDL_Rect box = {preview_x - 10, preview_y - 10, 110, 110}; //riquadro della preview
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255); //bordo grigio del riquadro
    SDL_RenderDrawRect(renderer, &box); //disegna il riquadro esterno

    for (int i = 0; i < BLOCKS_PER_PIECE; i++){
        SDL_Rect rect;
        rect.x = preview_x + game->next_piece.shape[i].dx * preview_block;
        rect.y = preview_y + game->next_piece.shape[i].dy * preview_block;
        rect.w = preview_block; //larghezza blocco preview
        rect.h = preview_block; //altezza blocco preview

        draw_block_3d(renderer, rect, game->next_piece.color_id, 0);
    }
}

//questa funzione blocca il pezzo attuale e ne manda uno nuovo
//controlla però che la prima riga non sia occupata o che il nuovo pezzo entri completamente
//in caso contrario blocca la partita, mostra un messaggio e ne comincia una nuova
void game_over(game_state* game, SDL_Window* window){
    lock_piece(game); //blocca prima il pezzo attuale

    if (top_row_occupied(game)){
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Game over", "pezzo fisso in prima riga, ricomincia", window);
        init_game(game);
        return;
    }
    clear_lines(game);
    spawn_piece(game);

    //controlla che il nuovo pezzo entri
    if (!can_place(game, game->active_piece, game->active_x, game->active_y)){
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Game Over", "Non c'è spazio, ricomincia", window);
        init_game(game);
        return;
    }
}


int main(int argc, char* args[]){
    srand((unsigned int)time(NULL)); //serve a non generare sempre la stessa sequenza a ogni esecuzione

    //test per vedere se il terminale stampa correttamente
    game_state game;
    init_game(&game);

    printf("il gioco esiste e il punteggio iniziale è: %d\n", game.score);

    //test di prova stampa blocco
    //game.board[10][5] = 1;
    //game.board[19][0] = 1;

    //inizializzazione di SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("Errore di inizializzazione: %s\n", SDL_GetError());
        return 1;
    }

    //inizializzazione del modulo SDL_tff, per usare i font TrueType
    if (TTF_Init()== -1){
        printf("Errore di inizializzazione SDL_ttf: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    //Creazione finestra
    SDL_Window* window = SDL_CreateWindow("Progetto", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
            SCREEN_HEIGHT, SDL_WINDOW_SHOWN); //SDL_WINDOWPOS_UNDEFINED dice che sarà il SO a decidere dove mettere la finestra
    
    if (window == NULL){
        printf("errore di creazione della finestra %s\n", SDL_GetError());
        return 1;
    }


    //Creazione renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); //l'ultimo parametro usa la gpu per rendere il tutto più veloce

    if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)){
        printf("Errore SDL_image: %s\n", IMG_GetError());
    }

    SDL_Texture* bg_texture = IMG_LoadTexture(renderer, "bg-neon.png");


    //caricamento del font, 24 è la dimensione del testo
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
    //se non viene trovato, chiudo il programma
    if (font == NULL){
        printf("Errore caricamento font: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    //Loop
    bool quit = false;
    SDL_Event e;

    Uint64 last_drop_time = SDL_GetTicks64();


    while(!quit){
        Uint64 frame_start = SDL_GetTicks64();

        while(SDL_PollEvent(&e) != 0){ //check per vedere se ci sono stati eventi
            if (e.type == SDL_QUIT){ //se l'evento è la 'X' per chiudere la finestra
                quit = true;
            } else if(e.type == SDL_KEYDOWN){ 
                if (e.key.keysym.sym == SDLK_ESCAPE) quit = true; //se premi ESC per uscire
             else if (e.key.keysym.sym == SDLK_LEFT){ //se viene premuta la freccia a sinistra, controlla la collisione e poi sposta il pezzo
                if (can_place(&game, game.active_piece, game.active_x - 1, game.active_y))
                    game.active_x--;
            } else if(e.key.keysym.sym == SDLK_RIGHT){ //stessa cosa nel caso in cui viene premuta quella a destra
                if (can_place(&game, game.active_piece, game.active_x + 1, game.active_y))
                    game.active_x++;
            } else if(e.key.keysym.sym == SDLK_DOWN){ //stessa cosa nel caso in cui viene premuta la freccia verso il bassso
                if (can_place(&game, game.active_piece, game.active_x, game.active_y + 1)){
                    game.active_y++;
                } else { //se can_place è false, allora blocca il pezzo e ne fa uscire uno nuovo
                    game_over(&game, window);
                }
            } else if(e.key.keysym.sym == SDLK_UP){ //se viene premuta la freccia verso l'alto, il pezzo viene ruotato secondo la funzione rotate_piece
                Piece rotated = rotate_piece(game.active_piece);
                if (can_place(&game, rotated, game.active_x, game.active_y))
                    game.active_piece = rotated; //il nuovo pezzo attivo diventa quello ruotato
            }
            }
        }
        Uint64 current_time = SDL_GetTicks64();

        if (current_time - last_drop_time >= DROP_INTERVAL){
            if (can_place(&game, game.active_piece, game.active_x, game.active_y + 1)){
                game.active_y++;
            } else {  //se can_place è false, allora blocca il pezzo e ne fa uscire uno nuovo
                game_over(&game, window);
            }
            last_drop_time = current_time;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); //colore nero
        SDL_RenderClear(renderer); //cancella lo schermo con il colore nero

        if (bg_texture){
            SDL_RenderCopy(renderer, bg_texture, NULL, NULL);
        } else {
            SDL_SetRenderDrawColor(renderer, 6, 10, 28, 255);
            SDL_RenderClear(renderer);
        }
        draw_board_frame(renderer);
        draw_board(renderer, &game);
        draw_active_piece(renderer, &game);
        draw_sidebar(renderer);
        draw_next_piece(renderer, &game);
        draw_score(renderer, font, &game);


        SDL_RenderPresent(renderer); //mostra il risultato

        Uint64 frame_time = SDL_GetTicks64() - frame_start;
        //se il tempo passato è inferiore a FRAME_DELAY aspetto comunque che finiscano i 16ms
        if (frame_time < FRAME_DELAY){
            SDL_Delay((Uint32)(FRAME_DELAY - frame_time));
        }
    }

    //Pulizia (chiusura e distruzione di window e renderer)
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_DestroyTexture(bg_texture);
    IMG_Quit();
    SDL_Quit();

    return 0;
}