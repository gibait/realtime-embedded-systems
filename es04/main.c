//
// Created by giba on 11/04/23.
//

#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

#define CARTA 0
#define SASSO 1
#define FORBICE 2

char *moves[3] = {"paper", "rock", "scissor"};

struct gestore_t {
	sem_t mutex;

	sem_t inizio_partita;
	sem_t fine_partita;

	int mossa_g1, mossa_g2;
	int numero_mosse;
} gestore;

void init_gestore(struct gestore_t *gestore) {
	sem_init(&gestore->mutex, 0, 1);
	sem_init(&gestore->inizio_partita, 0, 0);
	sem_init(&gestore->fine_partita, 0, 0);

	gestore->mossa_g1 = -1;
	gestore->mossa_g2 = -1;

	gestore->numero_mosse = 0;
}

int controlla_vincitore(int p1, int p2) {
	if (p1 == p2) {
		return -1;
	}

	switch (p1) {
	case CARTA:
		switch (p2) {
		case SASSO:
			return p1;
		case FORBICE:
			return p2;
		default:
			return -1;
		}
	case SASSO:
		switch (p2) {
		case CARTA:
			return p2;
		case FORBICE:
			return p2;
		default:
			return -1;
		}
	case FORBICE:
		switch (p2) {
		case CARTA:
			return p1;
		case SASSO:
			return p2;
		default:
			return -1;
		}
	default:
		return -1;
	}
}

void inizio_arbitro() {
	sem_wait(&gestore.mutex);

	sem_post(&gestore.inizio_partita);
	sem_post(&gestore.inizio_partita);

	printf("Inizia la partita..\n");

	sem_post(&gestore.mutex);
	sem_wait(&gestore.fine_partita);
}

void fine_arbitro() {
	int vincitore;

	sem_wait(&gestore.mutex);

	vincitore = controlla_vincitore(gestore.mossa_g1, gestore.mossa_g2);
	if (vincitore == -1) {
		printf("PATTA\n");
	} else if (vincitore == gestore.mossa_g1) {
		printf("Giocatore 1 vincitore!\n");
	} else if (vincitore == gestore.mossa_g2) {
		printf("Giocatore 2 vincitore!\n");
	} else {
		printf("Qualcosa è andato storto\n");
	}

	gestore.mossa_g1 = -1;
	gestore.mossa_g2 = -1;

	sem_post(&gestore.mutex);
}

void giocatore1() {
	sem_wait(&gestore.inizio_partita);

	gestore.mossa_g1 = rand() % 3;

	printf("G1: %s\n", moves[gestore.mossa_g1]);

	sem_wait(&gestore.mutex);
	if (++gestore.numero_mosse == 2) {
		sem_post(&gestore.fine_partita);
		gestore.numero_mosse = 0;
	}
	sem_post(&gestore.mutex);
}

void giocatore2() {
	sem_wait(&gestore.inizio_partita);

	gestore.mossa_g2 = rand() % 3;

	printf("G2: %s\n", moves[gestore.mossa_g2]);

	sem_wait(&gestore.mutex);
	if (++gestore.numero_mosse == 2) {
		sem_post(&gestore.fine_partita);
		gestore.numero_mosse = 0;
	}
	sem_post(&gestore.mutex);
}

void *giocatore(void *args) {
	while (1)
	{
		giocatore1();
		giocatore2();

	}
}

void *arbitro(void *args) {
	while(1) {

		inizio_arbitro();
		fine_arbitro();

	}
}

int main (int argc, char **argv) {
	pthread_t thread;

	init_gestore(&gestore);

	pthread_create(&thread, NULL, giocatore, NULL);
	pthread_create(&thread, NULL, arbitro, NULL);

	sleep(10);

	return 0;
}