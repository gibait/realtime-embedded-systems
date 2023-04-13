//
// Created by giba on 13/04/23.
//

#include <semaphore.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define NUMERO_CORRIDORI 150

struct corsa_t {
	sem_t mutex;

	sem_t pronti, partenza, arrivo;

	int corridori_pronti, corridori_arrivati;
	int primo, ultimo;
} corsa;

void init_corsa(struct corsa_t *c)
{
	sem_init(&c->mutex, 0, 1);
	sem_init(&c->pronti, 0, 0);
	sem_init(&c->partenza, 0, 0);
	sem_init(&c->arrivo, 0, 0);

	c->corridori_pronti = c->corridori_arrivati = c->primo = c->ultimo = 0;
}

void arbitro_attendicorridori(struct corsa_t *c)
{
	sem_wait(&c->pronti);
	printf("arbitro> i corridori sono pronti\n");
}

void arbitro_via(struct corsa_t *c)
{
	// mutex lock is not necessary
	while (c->corridori_pronti --> 0) {
		sem_post(&c->partenza);
	}
	printf("arbitro> faccio partire i corridori\n");
}

void arbitro_risultato(struct corsa_t *c)
{
	sem_wait(&c->arrivo);

	sem_wait(&c->mutex);
	printf("arbitro> il 1° classificato è: %d\n", c->primo);
	printf("arbitro> l'ultimo classificato è: %d\n", c->ultimo);
	sem_post(&c->mutex);
}

void corridore_attendivia(struct corsa_t *c, int numerocorridore)
{
	sem_wait(&c->mutex);

	if (++c->corridori_pronti == NUMERO_CORRIDORI) {
		sem_post(&c->pronti);
	}

	printf("corridore-%d> sono arrivato alla partenza\n", numerocorridore);

	sem_post(&c->mutex);
	sem_wait(&c->partenza);

	printf("corridore-%d> inizio a correre\n", numerocorridore);
}

void corridore_arrivo(struct corsa_t *c, int numerocorridore)
{
	sem_wait(&c->mutex);

	if (++c->corridori_arrivati == NUMERO_CORRIDORI) {
		sem_post(&c->arrivo);
		c->ultimo = numerocorridore;
	} else if (c->corridori_arrivati == 1) {
		c->primo = numerocorridore;
	}

	printf("corridore-%d> sono giunto all'arrivo in pos. %d\n", numerocorridore, c->corridori_arrivati);

	sem_post(&c->mutex);
}

void *corridore(void *arg)
{
	int numerocorridore = *((int*) arg);
	// vado sulla pista
	corridore_attendivia(&corsa, numerocorridore);
	// corro più veloce possibile
	corridore_arrivo(&corsa, numerocorridore);
	// torno a casa
	pthread_exit(0);
}

void *arbitro(void *arg)
{
	// vado sulla pista
	arbitro_attendicorridori(&corsa);
	// pronti, attenti..
	arbitro_via(&corsa);
	// attendo che arrivino al termine
	arbitro_risultato(&corsa);
	pthread_exit(0);
}

void pausetta(void)
{
	struct timespec t;
	t.tv_sec = 0;
	t.tv_nsec = (rand()%10+1)*1000000;
	nanosleep(&t,NULL);
}

int main (int argc, char **argv) {
	pthread_t t_arbitro, thread;

	init_corsa(&corsa);

	pthread_create(&t_arbitro, NULL, arbitro, NULL);

	for (int i = 0; i < NUMERO_CORRIDORI; i++) {
		pthread_create(&thread, NULL, corridore, (void*) &i);
		pausetta();
	}

	pthread_join(t_arbitro, NULL);

	return 0;
}