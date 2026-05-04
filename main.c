#include "game.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

//Dimensioni finestra
const int SCREEN_WIDTH = 400;
const int SCREEN_HEIGHT = 600;

#define T_SIZE 30 //numero di pixel che occupa un blocco
#define DROP_INTERVAL 1000 //intervallo di 1000 ms (ogni quanto il pezzo scende verso il basso)

//funzione che disegna la griglia con i pezzi fissi
void draw_board(SDL_Renderer* renderer, game_state* game){
    for (int i = 0; i < ROWS; i++){
        for (int j = 0; j < COLS; j++){
            SDL_Rect rect;
            rect.x = j * T_SIZE; //il pixel in cui comincia il quadratino (in colonna)
            rect.y = i * T_SIZE; //il pixel in cui comincia il quadratino (in riga)
            rect.w = T_SIZE; //larghezza del quadratino
            rect.h = T_SIZE; //altezza del quadratino

            if (game->board[i][j] == 0){ //se la cella è vuota disegna solo il bordo grigio
                SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
                SDL_RenderDrawRect(renderer, &rect);
            } else { //se è piena disegna un blocco rosso e il bordo nero
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderFillRect(renderer, &rect); 

                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &rect);
            }
        }
    }
}

//funzione che disegna un pezzo sulla griglia che dovrà muoversi
void draw_active_piece(SDL_Renderer* renderer, game_state* game){
    SDL_SetRenderDrawColor(renderer, 252, 0, 252, 255);

    for (int i = 0; i < 5; i++){
        SDL_Rect rect;

        rect.x = (game->active_x + game->active_piece.shape[i].dx) * T_SIZE;
        rect.y = (game->active_y + game->active_piece.shape[i].dy) * T_SIZE;
        rect.w = T_SIZE;
        rect.h = T_SIZE;

        SDL_RenderFillRect(renderer, &rect);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 252, 0, 252, 255);
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
    game.board[10][5] = 1;
    //game.board[19][0] = 1;

    //inizializzazione di SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("Errore di inizializzazione: %s\n", SDL_GetError());
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

    //Loop
    bool quit = false;
    SDL_Event e;

    Uint64 last_drop_time = SDL_GetTicks64();


    while(!quit){
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

        draw_board(renderer, &game);
        draw_active_piece(renderer, &game);

        SDL_RenderPresent(renderer); //mostra il risultato
    }

    //Pulizia (chiusura e distruzione di window e renderer)
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}