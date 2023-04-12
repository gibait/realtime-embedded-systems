//
// Created by giba on 12/04/23.
//

#include <semaphore.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define N_TORTE 3
#define N_CLIENTI 3

#define do_something() usleep(rand() % 1000 + 1000)

struct pasticceria_t {
	sem_t mutex;
	sem_t torta_mancante, torta_cucinata, torta_preparata;

	int torte;

} pasticceria;

void init_pasticceria(struct pasticceria_t *p)
{
	sem_init(&p->mutex, 0, 1);
	sem_init(&p->torta_mancante, 0, 0);
	sem_init(&p->torta_cucinata, 0, 0);
	sem_init(&p->torta_preparata, 0, 0);

	p->torte = 0;
}

void cuoco_inizio_torta(struct pasticceria_t *p)
{
	sem_wait(&p->mutex);

	if (p->torte < N_TORTE) {
		sem_post(&p->torta_mancante);
	}

	sem_post(&p->mutex);
	sem_wait(&p->torta_mancante);

	printf("cuoco> sto cucinando una torta\n");
}

void cuoco_fine_torta(struct pasticceria_t *p)
{
	sem_wait(&p->mutex);

	sem_post(&p->torta_cucinata);
	p->torte++;

	printf("cuoco> ho finito di cucinare una torta\n");

	sem_post(&p->mutex);
}

void commesso_prendo_torta(struct pasticceria_t *p)
{
	sem_wait(&p->mutex);

	if (p->torte > 0) {
		sem_post(&p->torta_cucinata);
	}

	sem_post(&p->mutex);
	sem_wait(&p->torta_cucinata);

	printf("commesso> sto prendendo una torta cucinata\n");
}

void commesso_vendo_torta(struct pasticceria_t *p)
{
	sem_wait(&p->mutex);

	sem_post(&p->torta_preparata);
	printf("commesso> ho venduto una torta preparata\n");

	if (--p->torte == 0) {
		sem_post(&p->torta_mancante);
		printf("commesso> siamo a corto di torte\n");

	}

	sem_post(&p->mutex);
}

void cliente_acquisto(struct pasticceria_t *p)
{
	sem_wait(&p->torta_preparata);

	printf("cliente> ho comprato una torta\n");
}

void *cuoco(void *arg)
{
	while(1) {
		cuoco_inizio_torta(&pasticceria);
		// preparo_torta
		do_something();
		cuoco_fine_torta(&pasticceria);
	}
}

void *commesso(void *arg)
{
	while(1) {
		commesso_prendo_torta(&pasticceria);
		// incarto torta
		do_something();
		commesso_vendo_torta(&pasticceria);
	}
}

void *cliente(void *arg)
{
	while(1) {
		do_something();
		cliente_acquisto(&pasticceria);
		do_something();
	}
}

int main (int argc, char **argv) {
	pthread_t thread_cuoco, thread_commesso, thread_cliente[N_CLIENTI];

	init_pasticceria(&pasticceria);

	pthread_create(&thread_cuoco, NULL, cuoco, NULL);
	pthread_create(&thread_commesso, NULL, commesso, NULL);

	for (int i = 0; i < N_CLIENTI; i++) {
		pthread_create(&thread_cliente[i], NULL, cliente, NULL);
	}

	sleep(60);

	return 0;
}