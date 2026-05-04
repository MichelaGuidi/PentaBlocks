# PentaBlocks

Progetto sviluppato in C con SDL2.

L'obiettivo è realizzare un gioco ispirato ai falling blocks, con gestione dei pezzi, movimento laterale, rotazione, caduta automatica, fissaggio nella griglia e cancellazione delle righe complete.

## Stato attuale

Funzionalità implementate finora:
- inizializzazione della board;
- generazione casuale dei pezzi;
- movimento del pezzo attivo a sinistra e a destra;
- rotazione del pezzo;
- caduta automatica tramite timer;
- lock del pezzo quando non può più scendere;
- game over con riavvio della partita;
- eliminazione delle righe complete;
- associazione di un colore ai pezzi.

## Struttura del progetto

```text
Progetto/
├── Makefile
├── include/
│   └── game.h
├── src/
│   ├── main.c
│   ├── game.c
│   ├── main.o
│   └── game.o
└── bin/
```

## Requisiti

Per compilare il progetto è necessario avere installato:
- gcc
- make
- SDL2

Su sistemi Linux è necessario il pacchetto di sviluppo di SDL2.

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

## Comandi di gioco

- Freccia sinistra: sposta il pezzo a sinistra
- Freccia destra: sposta il pezzo a destra
- Freccia giù: fa scendere il pezzo
- Freccia su: ruota il pezzo
- ESC: chiude il gioco

## Note

Il progetto è attualmente in sviluppo incrementale tramite commit separati, corrispondenti alle varie funzionalità aggiunte.
