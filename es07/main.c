//
// Created by giba on 12/04/23.
//

#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define DIMENSIONE_DIVANO 4
#define NUM_BARBIERI 3

#define SHAVING_ITERATIONS 1000
#define PAYING_ITERATIONS 500

#define do_something() usleep(rand() % 1000 + 1000)

struct barberia_t {
	sem_t mutex;

	sem_t barbiere, cassiere, divano;
} barberia;

void init_barberia(struct barberia_t *b)
{
	sem_init(&b->mutex, 0, 1);
	sem_init(&b->barbiere, 0, NUM_BARBIERI);
	sem_init(&b->cassiere, 0, 1);
	sem_init(&b->divano, 0, DIMENSIONE_DIVANO);
}

void cliente()
{
	printf("cliente-%ld> sto aspettando che si liberi un divano\n", pthread_self());
	// attendo che si liberi un posto sul divano
	sem_wait(&barberia.divano);
	printf("cliente-%ld> ho trovato posto, attendo un barbiere\n", pthread_self());
	// attendo che si liberi un barbiere
	sem_wait(&barberia.barbiere);
	printf("cliente-%ld> il barbiere inizia a tagliarmi i capelli\n", pthread_self());
	// libero un posto sul divano
	sem_post(&barberia.divano);
	// simulazione taglio barba
	for (int i = 0; i < SHAVING_ITERATIONS; i++) do_something();
	printf("cliente-%ld> il barbiere ha finito di tagliare\n", pthread_self());
	// libero un barbiere
	sem_post(&barberia.barbiere);
	printf("cliente-%ld> sto aspettando il cassiere\n", pthread_self());
	// attendo che si liberi il cassiere
	sem_wait(&barberia.cassiere);
	for (int i = 0; i < PAYING_ITERATIONS; i++) do_something();
	printf("cliente-%ld> ho finito di pagare, esco dal negozio\n", pthread_self());
	// libero la cassa
	sem_post(&barberia.cassiere);
	printf("cliente-%ld> sono uscito\n", pthread_self());
}


void *cliente_thread(void *arg)
{
	while(1) {
		cliente();
	}
}

int main (int argc, char **argv) {
	pthread_t thread;

	init_barberia(&barberia);

	for (int i = 0; i < 10; i++) {
		pthread_create(&thread, NULL, cliente_thread, NULL);
	}

	sleep(60);

	return 0;
}