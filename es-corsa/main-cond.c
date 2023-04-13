//
// Created by giba on 13/04/23.
//

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define NUMERO_CORRIDORI 150

typedef enum {false, true} bool;

struct corsa_t {
	pthread_mutex_t mutex;

	pthread_cond_t pronti, partenza, arrivo;

	int corridori_pronti, corridori_arrivati;
	// boolean flag per la partenza
	bool via;
	int primo, ultimo;
} corsa;

void init_corsa(struct corsa_t *c)
{
	pthread_condattr_t c_attr;
	pthread_mutexattr_t m_attr;

	pthread_condattr_init(&c_attr);
	pthread_mutexattr_init(&m_attr);

	pthread_mutex_init(&c->mutex, &m_attr);
	pthread_cond_init(&c->pronti, &c_attr);
	pthread_cond_init(&c->partenza, &c_attr);
	pthread_cond_init(&c->arrivo, &c_attr);

	pthread_condattr_destroy(&c_attr);
	pthread_mutexattr_destroy(&m_attr);

	c->corridori_pronti = c->corridori_arrivati = c->primo = c->ultimo = 0;
	c->via = false;
}

void arbitro_attendicorridori(struct corsa_t *c)
{
	pthread_mutex_lock(&c->mutex);

	while(c->corridori_pronti != NUMERO_CORRIDORI) {
		pthread_cond_wait(&c->pronti, &c->mutex);
	}

	pthread_mutex_unlock(&c->mutex);
	printf("arbitro> i corridori sono pronti\n");
}

void arbitro_via(struct corsa_t *c)
{
	pthread_mutex_lock(&c->mutex);
	c->via = true;
	pthread_mutex_unlock(&c->mutex);
	pthread_cond_broadcast(&c->partenza);
	printf("arbitro> faccio partire i corridori\n");
}

void arbitro_risultato(struct corsa_t *c)
{
	pthread_mutex_lock(&c->mutex);
	while (c->corridori_arrivati != NUMERO_CORRIDORI) {
		pthread_cond_wait(&c->arrivo, &c->mutex);
	}

	printf("arbitro> il 1° classificato è: %d\n", c->primo);
	printf("arbitro> l'ultimo classificato è: %d\n", c->ultimo);
	pthread_mutex_unlock(&c->mutex);
}

void corridore_attendivia(struct corsa_t *c, int numerocorridore)
{
	pthread_mutex_lock(&c->mutex);

	if (++c->corridori_pronti == NUMERO_CORRIDORI) {
		pthread_cond_signal(&c->pronti);
	}

	printf("corridore-%d> sono arrivato alla partenza\n", numerocorridore);

	while (!c->via) {
		pthread_cond_wait(&c->partenza, &c->mutex);
	}

	pthread_mutex_unlock(&c->mutex);
	printf("corridore-%d> inizio a correre\n", numerocorridore);
}

void corridore_arrivo(struct corsa_t *c, int numerocorridore)
{
	pthread_mutex_lock(&c->mutex);

	if (++c->corridori_arrivati == NUMERO_CORRIDORI) {
		pthread_cond_signal(&c->arrivo);
		c->ultimo = numerocorridore;
	} else if (c->corridori_arrivati == 1) {
		c->primo = numerocorridore;
	}

	printf("corridore-%d> sono giunto all'arrivo in pos. %d\n", numerocorridore, c->corridori_arrivati);

	pthread_mutex_unlock(&c->mutex);
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