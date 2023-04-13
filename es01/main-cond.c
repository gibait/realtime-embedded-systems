//
// Created by giba on 13/04/23.
//

#include <pthread.h>
#include <stdlib.h>

#define STATO_INIZIALE 0
#define A 1
#define B 2
#define C 3
#define D_o_E 4
#define D 5
#define E 6

struct gestore_t {
	pthread_mutex_t mutex;

	pthread_cond_t a, b, c, d, e;

	int stato;

	int b_a, b_b, b_c, b_d, b_e;
	int n_b;
} gestore;

void startA()
{
	pthread_mutex_lock(&gestore.mutex);

	while (gestore.stato != STATO_INIZIALE) {
		gestore.b_a++;
		pthread_cond_wait(&gestore.a, &gestore.mutex);
		gestore.b_a--;
	}

	gestore.stato = A;

	pthread_mutex_unlock(&gestore.mutex);
}

void endA()
{
	pthread_mutex_lock(&gestore.mutex);

	gestore.stato = B;

	if (gestore.b_b) {
		pthread_cond_broadcast(&gestore.b);
	}

	pthread_mutex_unlock(&gestore.mutex);
}

void startB()
{
	pthread_mutex_lock(&gestore.mutex);

	while (gestore.stato != B) {
		gestore.b_b++;
		pthread_cond_wait(&gestore.b, &gestore.mutex);
		gestore.b_b--;
	}

	gestore.n_b++;

	pthread_mutex_unlock(&gestore.mutex);
}

void endB()
{
	pthread_mutex_lock(&gestore.mutex);

	gestore.n_b--;
	if (gestore.n_b == 0) {
		if (gestore.b_c > 0) {
			pthread_cond_signal(&gestore.c);
		} else if (gestore.b_a > 0) {
			pthread_cond_signal(&gestore.a);
		}
		gestore.stato = STATO_INIZIALE;
	}

	pthread_mutex_unlock(&gestore.mutex);
}

void startC()
{
	pthread_mutex_lock(&gestore.mutex);

	while(gestore.stato != STATO_INIZIALE) {
		gestore.b_c++;
		pthread_cond_wait(&gestore.c, &gestore.mutex);
		gestore.b_c--;
	}

	gestore.stato = C;

	pthread_mutex_unlock(&gestore.mutex);
}

void endC()
{
	pthread_mutex_lock(&gestore.mutex);

	if (gestore.b_d > 0) {
ì		pthread_cond_signal(&gestore.e);
	} else if (gestore.b_e > 0) {
		pthread_cond_signal(&gestore.e);
	}

	gestore.stato = D_o_E;

	pthread_mutex_unlock(&gestore.mutex);
}

void startD()
{
	pthread_mutex_lock(&gestore.mutex);

	while(gestore.stato != D_o_E) {
		gestore.b_d++;
		pthread_cond_wait(&gestore.d, &gestore.mutex);
		gestore.b_d--;
	}

	gestore.stato = D;

	pthread_mutex_unlock(&gestore.mutex);
}

void endD()
{
	pthread_mutex_lock(&gestore.mutex);

	if (gestore.b_c > 0) {
		pthread_cond_signal(&gestore.c);
	} else if (gestore.b_a > 0) {
		pthread_cond_signal(&gestore.a);
	}
	gestore.stato = STATO_INIZIALE;

	pthread_mutex_unlock(&gestore.mutex);
}

void startE()
{
	pthread_mutex_lock(&gestore.mutex);

	while(gestore.stato != D_o_E) {
		gestore.b_e++;
		pthread_cond_wait(&gestore.e, &gestore.mutex);
		gestore.b_e--;
	}

	gestore.stato = E;

	pthread_mutex_unlock(&gestore.mutex);
}

void endE()
{
	pthread_mutex_lock(&gestore.mutex);

	if (gestore.b_c > 0) {
		pthread_cond_signal(&gestore.c);
	} else if (gestore.b_a > 0) {
		pthread_cond_signal(&gestore.a);
	}
	gestore.stato = STATO_INIZIALE;

	pthread_mutex_unlock(&gestore.mutex);
}
