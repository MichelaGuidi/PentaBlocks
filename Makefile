#nome dell'eseguibile
TARGET = bin/progetto

#compilatore e flag di compilazione
#-Wall abilita gli avvisi comuni
#-Wextra abilita avvisi aggiuntivi
#-g include informazioni di debug
#-I./include dice al compilatore di cercare i file .h nella directory include
CC = gcc
CFLAGS = -Wall -Wextra -g -I./include $(shell pkg-config --cflags sdl2)

#librerie SDL2 per il linking
LDLIBS = $(shell pkg-config --libs sdl2)

#file sorgenti
SRCS = src/main.c src/game.c

#file oggetto creati dopo la compilazione
OBJS = $(SRCS:.c=.o)

#regola principale
all: $(TARGET)

#regola per compilare e linkare l'eseguibile
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDLIBS)

#regola per compilare i file sorgenti
src/%.o: src/%.c include/game.h
	$(CC) $(CFLAGS) -c $< -o $@

#crea la directory bin se non esiste
bin: 
	mkdir -p bin

#regola per rimuovere i file compilati
clean:
	rm -f src/*.o $(TARGET)

#regola per eseguire il programma
run: all
	./$(TARGET)

.PHONY: all clean run