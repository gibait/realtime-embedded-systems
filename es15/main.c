//
// Created by giba on 13/04/23.
//

#include <pthread.h>

struct conto_t {
	pthread_mutex_t mutex;

	pthread_cond_t preleva;

	int saldo;
} conto;

void deposita(struct conto_t *c, int versamento)
{
	if (versamento <= 0) {
		return;
	}
	
	pthread_mutex_lock(&c->mutex);

	c->saldo += versamento;

	pthread_cond_broadcast(&c->preleva);

	pthread_mutex_unlock(&c->mutex);
}

void preleva(struct conto_t *c, int prelievo)
{
	if (prelievo == 0) {
		return;
	}

	pthread_mutex_lock(&c->mutex);

	if (prelievo > c->saldo) {
		while (prelievo > c->saldo) {
			pthread_cond_wait(&c->preleva, &c->mutex);
		}
	}

	c->saldo -= prelievo;

	pthread_mutex_unlock(&c->mutex);
}
