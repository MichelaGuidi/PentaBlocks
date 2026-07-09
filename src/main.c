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

#define SIDEBAR_X 430
#define SIDEBAR_Y 30
#define SIDEBAR_W 300

#define STATS_BOX_H 220
#define NEXT_BOX_Y 280
#define NEXT_BOX_H 250

//componenti del menu
typedef enum {
    MENU_RESUME,
    MENU_RESTART,
    MENU_QUIT,
    MENU_ITEMS_COUNT
} PauseMenuItem;

//funzione che modifica la grafica di un rect passato come argomento
//grafica simile a quella della board con le cornici
void draw_hud_box(SDL_Renderer* renderer, SDL_Rect rect) {
    SDL_Rect glow = {rect.x - 4, rect.y - 4, rect.w + 8, rect.h + 8};

    SDL_SetRenderDrawColor(renderer, 90, 140, 255, 20);
    SDL_RenderFillRect(renderer, &glow);

    SDL_SetRenderDrawColor(renderer, 10, 16, 40, 230);
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, 95, 145, 255, 255);
    SDL_RenderDrawRect(renderer, &rect);

    SDL_Rect inner = {rect.x + 2, rect.y + 2, rect.w - 4, rect.h - 4};
    SDL_SetRenderDrawColor(renderer, 150, 80, 220, 70);
    SDL_RenderDrawRect(renderer, &inner);
}

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

//funzione che scrive il punteggio
void draw_score(SDL_Renderer* renderer, TTF_Font* font1, TTF_Font* font2, game_state* game){
    SDL_Color title = {180, 210, 255, 255};
    SDL_Color white = {245, 245, 255, 255};
    SDL_Color pink = {255, 130, 220, 255};
    SDL_Color cyan = {90, 220, 255, 255};

    char score_text[32];
    //costruisce una stringa contenente il punteggio corrente
    snprintf(score_text, sizeof(score_text), "%d", game->score);

    draw_text(renderer, font1, "PUNTEGGIO", SIDEBAR_X + 28, SIDEBAR_Y + 22, title);
    draw_text(renderer, font2, score_text, SIDEBAR_X + 28, SIDEBAR_Y + 65, white);

    SDL_SetRenderDrawColor(renderer, 170, 70, 220, 120);
    SDL_RenderDrawLine(renderer, SIDEBAR_X + 20, SIDEBAR_Y + 135, SIDEBAR_X + SIDEBAR_W - 20, SIDEBAR_Y + 135);

    draw_text(renderer, font1, "LIVELLO", SIDEBAR_X + 28, SIDEBAR_Y + 150, pink);
    draw_text(renderer, font1, "01", SIDEBAR_X + SIDEBAR_W - 55, SIDEBAR_Y + 150, pink);

    SDL_SetRenderDrawColor(renderer, 170, 70, 220, 90);
    SDL_RenderDrawLine(renderer, SIDEBAR_X + 20, SIDEBAR_Y + 178, SIDEBAR_X + SIDEBAR_W - 20, SIDEBAR_Y + 178);

    draw_text(renderer, font1, "LINEE", SIDEBAR_X + 28, SIDEBAR_Y + 185, cyan);
    draw_text(renderer, font1, "00", SIDEBAR_X + SIDEBAR_W - 55, SIDEBAR_Y + 185, cyan);
}

//funzione che disegna bordo della griglia
void draw_board_frame(SDL_Renderer* renderer){
    //simula un effetto glow sulla cornice della board
    SDL_Rect glow1 = {BOARD_OFFSET_X - 14, BOARD_OFFSET_Y - 14, COLS * T_SIZE + 28, ROWS * T_SIZE + 28};

    SDL_Rect glow2 = {BOARD_OFFSET_X - 18, BOARD_OFFSET_Y - 18, COLS * T_SIZE + 36, ROWS * T_SIZE + 36};

    SDL_SetRenderDrawColor(renderer, 90, 140, 255, 35);
    SDL_RenderFillRect(renderer, &glow1);

    SDL_SetRenderDrawColor(renderer, 180, 70, 255, 20);
    SDL_RenderFillRect(renderer, &glow2);

    SDL_Rect outer = {BOARD_OFFSET_X - 10, BOARD_OFFSET_Y - 10, COLS*T_SIZE + 20, ROWS* T_SIZE + 20};
    SDL_Rect inner = {BOARD_OFFSET_X - 4, BOARD_OFFSET_Y - 4, COLS*T_SIZE + 8, ROWS* T_SIZE + 8};
    
    //pannello scuro dietro la board
    SDL_SetRenderDrawColor(renderer, 8, 14, 36, 255);
    SDL_RenderFillRect(renderer, &outer);

    //bordo esterno freddo
    SDL_SetRenderDrawColor(renderer, 95, 145, 255, 255);
    SDL_RenderDrawRect(renderer, &outer);

    //bordo interno viola
    SDL_SetRenderDrawColor(renderer, 150, 80, 220, 110);
    SDL_RenderDrawRect(renderer, &inner);

    SDL_Rect base_glow ={ BOARD_OFFSET_X - 6, BOARD_OFFSET_Y + ROWS*T_SIZE +6, COLS* T_SIZE+12, 8};
    SDL_SetRenderDrawColor(renderer, 120, 50, 220, 70);
    SDL_RenderFillRect(renderer, &base_glow);
}

//funzione che disegna il pulsante del menu
void draw_pause_button(SDL_Renderer* renderer, TTF_Font* font1, SDL_Rect rect, bool clicked){
    SDL_Color pause_button_color = clicked ? (SDL_Color){255, 180, 255, 255} : (SDL_Color){180,210, 255, 255}; //il colore cambia se il cursore è sul pulsante
    draw_hud_box(renderer, rect); //disegna il box

    //se il cursore è sopra, allora aggiunge un contorno extra
    if (clicked){
        SDL_SetRenderDrawColor(renderer, 255, 120, 220, 80);
        SDL_RenderDrawRect(renderer, &rect);
    }

    //scrivo ESC sopra per indicare che si può aprire premento ESC dalla tastiera
    draw_text(renderer, font1, "ESC", SCREEN_WIDTH -130, SCREEN_HEIGHT -70, pause_button_color);
}

//disegna due box a destra della board (modificata rispetto a prima perché aveva solo un riquadro)
void draw_sidebar(SDL_Renderer* renderer){
    SDL_Rect stats_box = {SIDEBAR_X, SIDEBAR_Y, SIDEBAR_W, STATS_BOX_H};
    SDL_Rect next_box = {SIDEBAR_X, NEXT_BOX_Y, SIDEBAR_W, NEXT_BOX_H};

    draw_hud_box(renderer, stats_box);
    draw_hud_box(renderer, next_box);
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

            if (game->board[i][j] == 0){ //se la cella è vuota la colora di un blu scuro e aggiunge un bordo
                SDL_SetRenderDrawColor(renderer, 10, 16, 40, 110);
                SDL_RenderFillRect(renderer, &rect);

                SDL_SetRenderDrawColor(renderer, 40, 65, 110, 120);
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

//funzione che disegna la cornice interna della preview
void draw_preview_frame(SDL_Renderer* renderer){
    SDL_Rect preview = {SIDEBAR_X + 35, NEXT_BOX_Y + 55, 230, 140};

    SDL_SetRenderDrawColor(renderer, 8, 12, 30, 235);
    SDL_RenderFillRect(renderer, &preview);

    SDL_SetRenderDrawColor(renderer, 80, 110, 200, 255);
    SDL_RenderDrawRect(renderer, &preview);

    SDL_Rect inner = {preview.x + 2, preview.y + 2, preview.w - 4, preview.h - 4};
    SDL_SetRenderDrawColor(renderer, 150, 80, 220, 60);
    SDL_RenderDrawRect(renderer, &inner);
}

//funzione che disegna il contenuto del pannello della preview
void draw_next_panel(SDL_Renderer* renderer, TTF_Font* font1){
    SDL_Color title_color = {180, 210, 255, 255};
    draw_text(renderer, font1, "PROSSIMO PEZZO", SIDEBAR_X + 28, NEXT_BOX_Y + 18, title_color);
    draw_preview_frame(renderer);
}

//funzione che disegna il next_piece a lato
void draw_next_piece(SDL_Renderer* renderer, game_state* game){
    int preview_x = SIDEBAR_X + 95; //inizio dell'area di preview a lato
    int preview_y = NEXT_BOX_Y + 85; //altezza della preview nella finestra
    int preview_block = 24; //blocchi più piccoli rispetto alla griglia principale

    for (int i = 0; i < BLOCKS_PER_PIECE; i++){
        SDL_Rect rect;
        rect.x = preview_x + game->next_piece.shape[i].dx * preview_block;
        rect.y = preview_y + game->next_piece.shape[i].dy * preview_block;
        rect.w = preview_block; //larghezza blocco preview
        rect.h = preview_block; //altezza blocco preview

        draw_block_3d(renderer, rect, game->next_piece.color_id, 0);
    }
}

//funzione che disdegna il menu
void draw_pause_menu(SDL_Renderer* renderer, TTF_Font* font1, TTF_Font* font2, int selected, int hovered){
    SDL_Rect box = {SCREEN_WIDTH/2 -170, SCREEN_HEIGHT/2 - 140, 360, 280}; //box che contiene il menu
    SDL_SetRenderDrawColor(renderer, 10, 16, 40, 240);
    SDL_RenderFillRect(renderer, &box);

    //bordo esterno del menù
    SDL_SetRenderDrawColor(renderer, 95, 145, 255, 255);
    SDL_RenderDrawRect(renderer, &box);

    //bordo interno per restare in tema con il gioco
    SDL_Rect inner = {box.x + 2, box.y + 2, box.w - 4, box.h - 4};
    SDL_SetRenderDrawColor(renderer, 150, 80, 220, 90);
    SDL_RenderDrawRect(renderer, &inner);

    //colori necessari
    SDL_Color title = {245, 245, 255, 255};
    SDL_Color normal = {180, 210, 255, 255};
    SDL_Color active = {255, 130, 220, 255};

    draw_text(renderer, font2, "PAUSA", box.x + 95, box.y + 28, title);

    //rettangoli cliccabili, necessari per capire se il cursore è sopra
    SDL_Rect resume_rect = {box.x + 70, box.y + 90, 200, 50};
    SDL_Rect restart_rect = {box.x + 70, box.y + 140, 200, 50}; 
    SDL_Rect quit_rect = {box.x + 70, box.y + 190, 200, 50};

    //posizione attuale del mouse
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    SDL_Point mouse = {mx, my};

    //se è sopra una delle voci, cambio il valore di hovered
    if (SDL_PointInRect(&mouse, &resume_rect)){
        hovered = MENU_RESUME;
    } else if (SDL_PointInRect(&mouse, &restart_rect)){
        hovered = MENU_RESTART;
    } else if (SDL_PointInRect(&mouse, &quit_rect)){
        hovered = MENU_QUIT;
    } else {
        hovered = -1; //se non è sopra niente allora torna neutro
    }

    //la voce è 'attiva' se sopra c'è il cursore oppure se è selezionato dalla tastiera
    bool resume_active = (hovered == MENU_RESUME) || (hovered == -1 && selected== MENU_RESUME);
    bool restart_active = (hovered == MENU_RESTART) || (hovered == -1 && selected == MENU_RESTART);
    bool quit_active = (hovered == MENU_QUIT) || (hovered == -1 && selected == MENU_QUIT);

    //disegno delle stringhe, con colori che dipendono dal fatto che il pulsante sia attivo o meno
    draw_text(renderer, resume_active ? font2 : font1, "Riprendi", box.x + 90, box.y + 95, resume_active? active : normal);
    draw_text(renderer, restart_active ? font2 : font1, "Ricomincia", box.x + 90, box.y + 145, restart_active? active : normal);
    draw_text(renderer, quit_active ? font2 : font1, "Esci", box.x + 90, box.y + 195, quit_active? active : normal);
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
    SDL_Rect pause_box = {SCREEN_WIDTH - 140, SCREEN_HEIGHT -80, 70, 50}; //riquadro che contiene il pulsante menu (pausa)

    Uint64 last_drop_time = SDL_GetTicks64();


    while(!quit){
        Uint64 frame_start = SDL_GetTicks64();

        while(SDL_PollEvent(&e) != 0){ //check per vedere se ci sono stati eventi
            if (e.type == SDL_QUIT){ //se l'evento è la 'X' per chiudere la finestra
                quit = true;
            } else if(e.type == SDL_KEYDOWN){
                hovered = -1;
                if (paused){ //se paused è true allora si trova nell'area menù
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
                } else { //caso in cui non si è in pausa, e si usa la tastiera
                    if (e.key.keysym.sym == SDLK_ESCAPE){
                        paused = true;
                        menu_selected = -1;
                        hovered = -1;
                    } else if (e.key.keysym.sym == SDLK_LEFT){
                         //se viene premuta la freccia a sinistra, controlla la collisione e poi sposta il pezzo
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
            } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT){ //se viene premuto il pulsante del mouse
                SDL_Point click_point = {e.button.x, e.button.y}; //punto click del mouse
                if (!paused){ //se non è nel menu
                    if (SDL_PointInRect(&click_point, &pause_box)){ //se il click è nel rettangolo di pausa
                        paused = true; //apre il menu
                        menu_selected = -1; 
                        hovered = -1;
                    }
                } else { // se il menu è già aperto
                    SDL_Rect menu_box = {SCREEN_WIDTH / 2 - 170, SCREEN_HEIGHT / 2 - 140, 340, 280};
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
                }
            }
        }
        Uint64 current_time = SDL_GetTicks64();

        //se il gioco non è in pausa e il tempo è scaduto, il pezzo prova a scendere
        if (!paused && current_time - last_drop_time >= DROP_INTERVAL){
            if (can_place(&game, game.active_piece, game.active_x, game.active_y + 1)){
                game.active_y++;
            } else {  //se can_place è false, allora blocca il pezzo e ne fa uscire uno nuovo
                game_over(&game, window);
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
        //disegno dell'interfaccia, chiamando in ordine tutte le funzioni
        draw_board_frame(renderer);
        draw_board(renderer, &game);
        draw_active_piece(renderer, &game);
        draw_sidebar(renderer);
        draw_next_panel(renderer, font1);
        draw_next_piece(renderer, &game);
        draw_score(renderer, font1, font2, &game);

        //rilevamento cursore sul pulsante
        int mx, my; //coordinate del mouse
        SDL_GetMouseState(&mx, &my); //posizioni del cursore nella finestra
        SDL_Point mouse_point = {mx, my}; //converte le coordinate in un SDL_point

        //se si trova sul pulsante di PAUSA
        bool clicked = SDL_PointInRect(&mouse_point, &pause_box); //se si trova sopra il pulsante

        draw_pause_button(renderer, font1, pause_box, clicked); //disegna il pulsante

        //se il menu è aperto, allora lo disegno
        if (paused){
            draw_pause_menu(renderer, font1, font2, menu_selected, hovered); //disegna il menu solo se è in pausa
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