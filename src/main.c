#include "game.h"
#include "config.h"
#include "render.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

int main(int argc, char* args[]){
    (void) argc;
    (void) args;

    srand((unsigned int)time(NULL)); //serve a non generare sempre la stessa sequenza a ogni esecuzione

    game_state game;

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

    //inizializzazione di SDL_image per png
    if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)){
        printf("Errore SDL_image: %s\n", IMG_GetError());
    }

    //caricamento dello sfondo
    SDL_Texture* bg_texture = IMG_LoadTexture(renderer, "bg-neon.png");


    //caricamento del font, 20 è la dimensione del testo
    TTF_Font* font1 = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 20);

    //aggiunta di un secondo font più grande per segnare il punteggio
    TTF_Font* font2 = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 42);
    //se non viene trovato, chiudo il programma
    if (font1 == NULL ||font2 == NULL){
        printf("Errore caricamento font: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    //Loop
    bool quit = false; //termina il loop principale se true
    SDL_Event e; //struttura che contiene gli eventi SDL
    bool paused = false; //true quando il menu pausa è aperto
    int menu_selected = -1; //selezione da tastiera del menu
    int hovered = -1; //selezione con il cursore del menu
    bool start_screen = true; //true solo all'avvio
    SDL_Rect pause_box = {SCREEN_WIDTH - 120, SCREEN_HEIGHT -80, 70, 50}; //pulsante per aprire il menu

    Uint64 last_drop_time = SDL_GetTicks64();
    Uint64 last_frame_time = SDL_GetTicks64(); //calcola quanto tempo passa tra un frame e il successivo


    while(!quit){
        Uint64 frame_start = SDL_GetTicks64();
        Uint64 now = SDL_GetTicks64(); //tempo attuale
        int delta_ms = (int)(now - last_frame_time); //millisecondi passati
        last_frame_time = now;

        if (game.flash_timer_ms > 0){ //se il flash è attivo, ne diminuisce gradualmente la durata residua
            game.flash_timer_ms -= delta_ms;
            if (game.flash_timer_ms < 0){ //evita che vada sotto lo 0
                game.flash_timer_ms = 0;
            }
        }

        while(SDL_PollEvent(&e) != 0){ //check per vedere se ci sono stati eventi
            if (e.type == SDL_QUIT){ //se l'evento è la 'X' per chiudere la finestra
                quit = true;
            } else if(e.type == SDL_KEYDOWN){ //se l'evento è la pressione di qualche pulsante sulla tastiera
                if (start_screen){
                    if (e.key.keysym.sym == SDLK_UP || e.key.keysym.sym == SDLK_DOWN){
                        if (menu_selected == -1) menu_selected = 0;
                        else if (menu_selected == 0) menu_selected = 1;
                        else if (menu_selected == 1) menu_selected = 0;
                    } else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER){
                        if (menu_selected == 0){
                            start_screen = false;
                            init_game(&game);
                            hovered = -1;
                            menu_selected = -1;
                            last_drop_time = SDL_GetTicks64();
                        } else {
                            quit = true;
                        }
                    } else if (e.key.keysym.sym == SDLK_ESCAPE){
                        quit = true;
                    }
                } else if (paused){ //se paused è true allora si trova nell'area menù
                    if (e.key.keysym.sym == SDLK_ESCAPE){ //se si preme esc allora si torna al gioco
                        paused = false;
                        menu_selected = -1; //reset della selezione da tastiera
                        hovered = -1; //reset della selezione da mouse
                        last_drop_time = SDL_GetTicks64();
                    } else if (e.key.keysym.sym == SDLK_UP){
                        if (menu_selected == -1) menu_selected = MENU_RESUME; //se nessuna voce è selezionata parte da 'riprendi'
                        else {
                            menu_selected--;
                            if (menu_selected < 0) menu_selected = MENU_ITEMS_COUNT - 1;
                        }
                    } else if (e.key.keysym.sym == SDLK_DOWN){
                        if (menu_selected == -1) menu_selected = MENU_RESUME;
                        else {
                            menu_selected++;
                            if (menu_selected >= MENU_ITEMS_COUNT) menu_selected = 0;
                        }
                    } else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER){
                        //è selezionata la voce 'riprendi'
                        if (menu_selected == MENU_RESUME){
                            paused = false;
                            menu_selected = -1;
                            hovered = -1;
                            last_drop_time = SDL_GetTicks64();
                        } else if (menu_selected == MENU_RESTART){ //è selezionata la voce 'ricomincia'
                            init_game(&game);
                            paused = false;
                            menu_selected = -1;
                            hovered = -1;
                            last_drop_time = SDL_GetTicks64();
                        } else if (menu_selected == MENU_QUIT){ //è selezionata la voce 'esci'
                            quit = true;
                        }
                    }
                } else if (game.game_over_screen){ //se la partita è stata persa
                    if (e.key.keysym.sym == SDLK_UP || e.key.keysym.sym == SDLK_DOWN){
                        if (menu_selected == -1) menu_selected = 0;
                        else if (menu_selected == 0) menu_selected = 1;
                        else if (menu_selected == 1) menu_selected = 0;
                    } else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER){
                        if (menu_selected == 0){
                            init_game(&game);
                            game.game_over_screen = false;
                            menu_selected = -1;
                            last_drop_time = SDL_GetTicks64();
                        } else {
                            quit = true;
                        }
                    } 
                } else if (game.level_complete){
                    if (e.key.keysym.sym == SDLK_UP){
                        if (menu_selected == -1) menu_selected = MENU_RESUME; //se nessuna voce è selezionata parte da 'Avanza'
                        else {
                            menu_selected--;
                            if (menu_selected < 0) menu_selected = MENU_ITEMS_COUNT - 1;
                        }
                    } else if (e.key.keysym.sym == SDLK_DOWN){ //se nessuna voce è selezionata allora si parte dal tasto 'Avanza'
                        if (menu_selected == -1) menu_selected = MENU_RESUME;
                        else {
                            menu_selected++;
                            if (menu_selected >= MENU_ITEMS_COUNT) menu_selected = 0;
                        }
                    } else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER){
                        //è selezionata la voce 'Avanza' e quindi vado al prossimo livello
                        if (menu_selected == MENU_RESUME){
                            start_next_level(&game);
                            last_drop_time = SDL_GetTicks64();
                        } else if (menu_selected == MENU_RESTART){ //è selezionata la voce 'ricomincia'
                            init_game(&game);
                            menu_selected = -1;
                            hovered = -1;
                            last_drop_time = SDL_GetTicks64();
                        } else if (menu_selected == MENU_QUIT){ //è selezionata la voce 'esci'
                            quit = true;
                        }
                    } 
                } else { //se è nella schermata di gioco
                    if (e.key.keysym.sym == SDLK_ESCAPE){ //premendo esc si apre il menu
                        paused = true;
                        menu_selected = -1;
                        hovered = -1;
                    } else if (e.key.keysym.sym == SDLK_LEFT){
                         //se viene premuta la freccia a sinistra, controlla la collisione e poi sposta il pezzo
                        if (can_place(&game, game.active_piece, game.active_x - 1, game.active_y))
                            game.active_x--;
                    } else if(e.key.keysym.sym == SDLK_RIGHT){ //stessa cosa nel caso in cui viene premuta quella a destra
                        if (can_place(&game, game.active_piece, game.active_x + 1, game.active_y)){
                            game.active_x++;
                        }
                    } else if(e.key.keysym.sym == SDLK_DOWN){ //stessa cosa nel caso in cui viene premuta la freccia verso il bassso
                        if (can_place(&game, game.active_piece, game.active_x, game.active_y + 1)){
                            game.active_y++;
                        } else { //se can_place è false, allora blocca il pezzo e ne fa uscire uno nuovo
                            game_over(&game);
                        }
                    } else if(e.key.keysym.sym == SDLK_UP){ //se viene premuta la freccia verso l'alto, il pezzo viene ruotato secondo la funzione rotate_piece
                        Piece rotated = rotate_piece(game.active_piece);
                        if (can_place(&game, rotated, game.active_x, game.active_y))
                            game.active_piece = rotated; //il nuovo pezzo attivo diventa quello ruotato
                    }
                }
            } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT){ //se viene premuto il pulsante del mouse
                SDL_Point click_point = {e.button.x, e.button.y}; //punto click del mouse
                if (game.level_complete){ //se il livello è stato completato appare il pulsante 'AVANZA' che diventa disponibile
                    SDL_Rect box = {SCREEN_WIDTH/2 -170, SCREEN_HEIGHT/2 - 140, 380, 320};
                    SDL_Rect next_button = {box.x + 85, box.y + 155, 210, 50};
                    SDL_Rect reset_button = {box.x + 85, box.y + 210, 210, 50};
                    SDL_Rect quit_button = {box.x + 85, box.y + 265, 210, 50};
                    if (SDL_PointInRect(&click_point, &next_button)){
                        start_next_level(&game);
                        last_drop_time = SDL_GetTicks64();
                    } else if(SDL_PointInRect(&click_point, &reset_button)){
                        init_game(&game);
                        menu_selected = -1;
                        hovered = -1;
                        last_drop_time = SDL_GetTicks64();
                    }else if (SDL_PointInRect(&click_point, &quit_button)){
                        quit = true;
                    }
                } else if (game.game_over_screen){ //se la partita è stata persa
                    SDL_Rect box = {SCREEN_WIDTH/2 -170, SCREEN_HEIGHT/2 - 140, 380, 280};
                    SDL_Rect restart_rect = {box.x + 50, box.y + 110, 250, 50};
                    SDL_Rect quit_rect = {box.x + 50, box.y + 160, 250, 50};

                    if (SDL_PointInRect(&click_point, &restart_rect)){
                        init_game(&game);
                        game.game_over_screen = false;
                        last_drop_time = SDL_GetTicks64();
                    } else if (SDL_PointInRect(&click_point, &quit_rect)){
                        quit = true;
                    }
                } else if (!paused && !start_screen){ //se non è nel menu
                    if (SDL_PointInRect(&click_point, &pause_box)){ //se il click è nel rettangolo di pausa
                        paused = true; //apre il menu
                        menu_selected = -1; 
                        hovered = -1;
                    }
                } else if (paused && !start_screen){ // se il menu è già aperto
                    SDL_Rect menu_box = {SCREEN_WIDTH / 2 - 170, SCREEN_HEIGHT / 2 - 140, 380, 280};
                    //rettangoli cliccabili
                    SDL_Rect resume_rect  = {menu_box.x + 70, menu_box.y + 90, 200, 40};
                    SDL_Rect restart_rect = {menu_box.x + 70, menu_box.y + 140, 200, 40};
                    SDL_Rect quit_rect    = {menu_box.x + 70, menu_box.y + 190, 200, 40};
                    //se clicco su 'riprendi'
                    if (SDL_PointInRect(&click_point, &resume_rect)){
                        paused = false;
                        menu_selected = -1;
                        hovered = -1;
                        last_drop_time = SDL_GetTicks64();
                    } else if (SDL_PointInRect(&click_point, &restart_rect)){ //se clicco su 'ricomincia'
                        init_game(&game);
                        paused = false;
                        menu_selected = -1;
                        hovered = -1;
                        last_drop_time = SDL_GetTicks64();
                    } else if (SDL_PointInRect(&click_point, &quit_rect)){ //se clicco su 'esci'
                        quit = true;
                    }
                } else if (start_screen) {
                    SDL_Rect box = {20, SCREEN_HEIGHT/2 - 140, 380, 280};
                    SDL_Rect init_rect = {box.x + 50, box.y + 110, 250, 50};
                    SDL_Rect quit_rect = {box.x + 50, box.y + 160, 250, 50};

                    if (SDL_PointInRect(&click_point, &init_rect)){
                        init_game(&game);
                        start_screen = false;
                        last_drop_time = SDL_GetTicks64();
                    } else if (SDL_PointInRect(&click_point, &quit_rect)){
                        quit = true;
                    }
                }
            }
        }
        Uint64 current_time = SDL_GetTicks64();

        //se il gioco non è in pausa/il livello non è finito e il tempo è scaduto, il pezzo prova a scendere
        if (!start_screen && !paused && !game.level_complete && !game.game_over_screen && current_time - last_drop_time >= (Uint64)get_drop_interval_ms(&game)){
            if (can_place(&game, game.active_piece, game.active_x, game.active_y + 1)){
                game.active_y++;
            } else {  //se can_place è false, allora blocca il pezzo e ne fa uscire uno nuovo
                game_over(&game);
            }
            last_drop_time = current_time;
        }
        //pulizia dello schermo
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); //colore nero
        SDL_RenderClear(renderer); //cancella lo schermo con il colore nero

        //disegno dello sfondo
        if (bg_texture){
            SDL_RenderCopy(renderer, bg_texture, NULL, NULL);
        } else {
            SDL_SetRenderDrawColor(renderer, 6, 10, 28, 255);
            SDL_RenderClear(renderer);
        }

        if (start_screen){ //se il programma è appena stato aperto, allora mostra il pannello di inizio gioco
            draw_first_box(renderer, font1, font2, menu_selected, hovered);
            draw_controls_box(renderer, font1, font2);
        } else{
            //disegno dell'interfaccia, chiamando in ordine tutte le funzioni
            draw_board_frame(renderer);
            draw_board(renderer, &game);
            draw_ghost_piece(renderer, &game);
            draw_line_clear_flash(renderer, &game);
            draw_active_piece(renderer, &game);
            draw_sidebar(renderer);
            draw_next_panel(renderer, font1);
            draw_next_piece(renderer, &game);
            draw_score(renderer, font1, font2, &game);
            draw_min_level_score(renderer, font1, &game);

            //rilevamento cursore sul pulsante
            int mx, my; //coordinate del mouse
            SDL_GetMouseState(&mx, &my); //posizioni del cursore nella finestra
            SDL_Point mouse_point = {mx, my}; //converte le coordinate in un SDL_point
            //se si trova sul pulsante di PAUSA
            bool clicked = SDL_PointInRect(&mouse_point, &pause_box); //se si trova sopra il pulsante
            draw_pause_button(renderer, font1, pause_box, clicked); //disegna il pulsante

            if (game.level_complete){//se il livello è stato completato allora disegno il box per avanzare di livello 
                draw_level_complete_box(renderer, font1, font2, &game, menu_selected, hovered);
            } else if (paused){ //se viene cliccato il pulsante di pausa/menu
                draw_pause_menu(renderer, font1, font2, menu_selected, hovered); //disegna il menu solo se è in pausa
            } //se il livello è stato perso allora disegno il pannello di game_over
            else if (game.game_over_screen){
                draw_game_over(renderer, font1, font2, menu_selected, hovered);
            }
        }

        SDL_RenderPresent(renderer); //mostra il risultato

        Uint64 frame_time = SDL_GetTicks64() - frame_start;
        //se il tempo passato è inferiore a FRAME_DELAY aspetto comunque che finiscano i 16ms
        if (frame_time < FRAME_DELAY){
            SDL_Delay((Uint32)(FRAME_DELAY - frame_time));
        }
    }

    //Pulizia (chiusura e distruzione di window e renderer)
    TTF_CloseFont(font1);
    TTF_CloseFont(font2);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_DestroyTexture(bg_texture);
    IMG_Quit();
    SDL_Quit();

    return 0;
}