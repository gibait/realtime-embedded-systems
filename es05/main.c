//
// Created by giba on 12/04/23.
//

#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define POPOLAZIONE 7

typedef enum {false, true} bool;

struct urna_t {
	sem_t mutex;

	sem_t fine_votazione;

	int voti_sx, voti_dx;
	int votanti;
} urna;

void init_urna(struct urna_t *urna) {
	sem_init(&urna->mutex, 0, 1);
	sem_init(&urna->fine_votazione, 0, 0);

	urna->voti_sx = urna->voti_sx = urna->votanti = 0;
}

void vota(bool voto) {
	sem_wait(&urna.mutex);

	if (voto) {
		urna.voti_dx++;
	} else {
		urna.voti_sx++;
	}

	urna.votanti++;

	// preemptive post
	if (urna.voti_sx >= POPOLAZIONE/2 || urna.voti_dx >= POPOLAZIONE/2) {
		sem_post(&urna.fine_votazione);
	}
	sem_post(&urna.mutex);
}

int risultato() {
	int retr;

	sem_wait(&urna.fine_votazione);
	sem_wait(&urna.mutex);

	if (urna.voti_sx > urna.voti_dx) {
		retr = 0;
	} else {
		retr = 1;
	}

	while (urna.votanti --> 0) {
		sem_post(&urna.fine_votazione);
	}
	sem_post(&urna.mutex);

	return retr;
}

void *exe_thread(void *arg)
{
	unsigned long votante = pthread_self();

	int voto = rand() % 2;
	vota(voto);
	printf("Votante-%ld: ho votato %d\n", votante, voto);
	if (voto == risultato()) printf("Votante-%ld: Ho vinto!\n", votante);
	else printf("Votante-%ld: Ho perso!\n", votante);
	pthread_exit(0);
}

int main (int argc, char **argv) {
	pthread_t thread;

	init_urna(&urna);

	for (int i = 0; i <= POPOLAZIONE; i++) {
		pthread_create(&thread, NULL, exe_thread, NULL);
	}

	return 0;
}