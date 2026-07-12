# PentaBlocks

PentaBlocks è un gioco sviluppato in C con SDL2, ispirato ai classici falling-block-games, ma basato su **pentamini** invece che su tetramini. Ogni pezzo è composto da 5 celle e il gioco combina logica arcade classica con un'interfaccia grafica futuristica.

## Descrizione

L'obiettivo è controllare la caduta dei pezzi, ruotarli e posizionarli nella board per completare righe orizzontali. Ogni riga completata viene eliminata, il punteggio aumenta e il gioco accelera progressivamente con l'aumento del livello.

## Funzionalità implementate

- board 10x20;
- 12 pezzi pentamino, ognuno composto da 5 blocchi;
- movimento laterale del pezzo attivo;
- rotazione del pezzo attivo;
- caduta automatica con velocità variabile in base al livello;
- soft drop con freccia giù;
- lock del pezzo quando non può più scendere;
- rilevamento collisione con bordi e blocchi fissi;
- eliminazione delle righe complete;
- sistema di punteggio;
- gestione dei livelli con soglie di avanzamento;
- preview del prossimo pezzo;
- schermata di pausa;
- schermata game over;
- schermata di completamento livello;
- gestione dei menu tramite tasto sinistro del mouse;
- interfaccia grafica laterale con punteggio, livello e linee;
- sfondo PNG e stile grafico neon;
- rendering dei blocchi con effetto pseudo-3D.

## Struttura del progetto

```text
Progetto/
├── Makefile
├── include/
│   └── game.h
│   └── render.h
│   └── config.h
├── src/
│   ├── main.c
│   ├── game.c
│   └── render.c
│   ├── main.o
│   └── game.o
│   └── render.o
└── bin/
```

### Ruolo dei file

- 'main.c': inizializzazione SDL, loop principale, input da tastiera/mouse, gestione degli stati di gioco;
- 'game.c': logica del gioco, spawn, collisioni, rotazione, lock, eliminazione righe, livelli;
- 'render.c': rendering di board, HUD, pannelli laterali, menu, pezzi e interfaccia;
- 'game.h': strutture dati e prototipi della logica di gioco;
- 'render.h': prototipi delle funzioni di rendering;
- 'config.h': costanti di configurazione per finestra, board e layout.


## Requisiti

Per compilare il progetto servono:

- gcc
- make
- SDL2
- SDL2_ttf
- SDL2_image

Su sistemi Linux è necessario installare i pacchetti di sviluppo delle librerie SDL.

## Compilazione

Per compilare il progetto:

```bash
make
```

L'eseguibile verrà creato nella cartella `bin/`.

## Esecuzione

Per eseguire il programma:

```bash
make run
```

oppure direttamente:

```bash
./bin/progetto
```

## Pulizia dei file compilati

Per rimuovere file oggetto ed eseguibile:

```bash
make clean
```

## Risorse esterne

Il programma utilizza:

- un'immagine PNG di sfondo ('bg-neon.png');
- font TrueType di sistema ('DejaVuSans' e 'DejaVuSans-Bold').

## Controlli

### Durante la partita

- Freccia sinistra: sposta il pezzo a sinistra
- Freccia destra: sposta il pezzo a destra
- Freccia giù: fa scendere il pezzo
- Freccia su: ruota il pezzo
- ESC: apre o chiude il menu di pausa

### Menu e schermate

- Freccia su/giù: cambia selezione
- Invio: conferma l'opzione selezionata
- Tasto sinistro del mouse: click sui pulsanti del menu

## Interfaccia

L'interfaccia è composta da:
 
- board principale con corince luminosa;
- pannello laterale con statistiche di punteggio, livello e righe completate;
- riquadro con preview del pezzo successivo;
- pulsante pausa;
- popup per pausa, game over e completamento livello.

## Note

Il progetto sviluppato tramite commit separati, corrispondenti alle varie funzionalità aggiunte.
