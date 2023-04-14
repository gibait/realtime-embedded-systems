//
// Created by giba on 14/04/23.
//

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NONE -1
#define RUNNERS 2

typedef enum {false, true} bool;

struct bandierine_t{
	pthread_mutex_t mutex;

	pthread_cond_t attendi, via, fine;

	int pronti;
	int giocatore_con_bandiera;
	bool salvo;
	int vincitore;

} bandierine;

void microPause(void) {
	struct timespec t;
	t.tv_sec = 0;
	t.tv_nsec = (rand() % + 1) * 1000000;
	nanosleep(&t, NULL);
}

void init_bandierine(struct bandierine_t *b)
{
	pthread_mutexattr_t mutex_attr;
	pthread_condattr_t cond_attr;

	pthread_mutexattr_init(&mutex_attr);
	pthread_condattr_init(&cond_attr);

	pthread_mutex_init(&b->mutex, &mutex_attr);
	pthread_cond_init(&b->attendi, &cond_attr);
	pthread_cond_init(&b->via, &cond_attr);
	pthread_cond_init(&b->fine, &cond_attr);

	pthread_mutexattr_destroy(&mutex_attr);
	pthread_condattr_destroy(&cond_attr);

	b->pronti = 0;
	b->giocatore_con_bandiera = b->vincitore = NONE;
	b->salvo = false;
}

/*
    Da il via al gioco sbloccando i giocatori in attesa
*/
void via(struct bandierine_t *b)
{
	pthread_cond_broadcast(&b->via);
}

/*
    Blocca i giocatori che attendono il via
*/
void attendi_il_via(struct bandierine_t *b, int n)
{
	pthread_mutex_lock(&b->mutex);

	while (b->pronti != 2) {
		if (b->pronti++ == 2) {
			pthread_cond_signal(&b->attendi);
		}
		pthread_cond_wait(&b->via, &b->mutex);
	}

	pthread_mutex_unlock(&b->mutex);
}

/*
    Prende la bandierina (se disponibile).
        1 -> Se il Giocatore prende la bandierina
        0 -> Se il Giocatore NON prende la bandierina
*/
int bandierina_presa(struct bandierine_t *b, int n)
{
	int retr;
	pthread_mutex_lock(&b->mutex);

	if (b->giocatore_con_bandiera == NONE) {
		b->giocatore_con_bandiera = n;
		retr = 1;
	} else {
		retr = 0;
	}

	pthread_mutex_unlock(&b->mutex);

	return retr;
}

/*
    1 -> Se il Giocatore NON è stato preso
    0 -> Se il Giocatore è stato preso
*/
int sono_salvo(struct bandierine_t *b, int n)
{
	int retr = 0;

	pthread_mutex_lock(&b->mutex);

	if (b->salvo == false) {
		b->salvo = true;
		retr = 1;
		b->vincitore = n;
	}

	pthread_cond_signal(&b->fine);

	pthread_mutex_unlock(&b->mutex);

	return retr;
}

/*
    1 -> Se il Giocatore NON si è ancora salvato
    0 -> Se il Giocatore si è già salvato
*/
int ti_ho_preso(struct bandierine_t *b, int n)
{
	int retr = 0;

	pthread_mutex_lock(&b->mutex);

	if (b->salvo == false) {
		retr = 1;
		b->vincitore = n;
		b->salvo = true;
	}

	pthread_cond_signal(&b->fine);

	pthread_mutex_unlock(&b->mutex);

	return retr;
}

/*
    Ritorna il Numero del Vincitore
*/
int risultato_gioco(struct bandierine_t *b)
{
	int retr;
	pthread_mutex_lock(&b->mutex);

	while (b->vincitore == NONE) {
		pthread_cond_wait(&b->fine, &b->mutex);
	}

	retr = b->vincitore;

	pthread_mutex_unlock(&b->mutex);

	return retr;
}

/*
    Attende che entrambi i giocatori siano pronti in attesa prima di dare il via
*/
void attendi_giocatori(struct bandierine_t *b)
{
	pthread_mutex_lock(&b->mutex);

	while (b->pronti != 2) {
		pthread_cond_wait(&b->attendi, &b->mutex);
	}

	pthread_mutex_unlock(&b->mutex);
}

void *giocatore(void *arg){
	int numero_giocatore = (int) arg;
	attendi_il_via(&bandierine, numero_giocatore);
	// corri e tenta di prendere la bandierina
	if (bandierina_presa(&bandierine, numero_giocatore)){
		printf("P%d: Ho preso la bandierina!\n", numero_giocatore);
		// corri alla base
		microPause();
		if (sono_salvo(&bandierine, numero_giocatore))
			printf("P%d: Sono Salvo!\n", numero_giocatore);
		else
			printf("P%d: Non sono riuscito a salvarmi!\n", numero_giocatore);
	} else {
		printf("P%d: Non sono riuscito a prendere la bandierina!\n", numero_giocatore);
		// cerca di acchiappare l'altro giocatore
		microPause();
		if (ti_ho_preso(&bandierine, numero_giocatore))
			printf("P%d: Ti ho preso!\n", numero_giocatore);
		else
			printf("P%d: Non sono riuscito a prenderti!\n", numero_giocatore);
	}

	pthread_exit(0);
}

void *giudice(void *arg){
	attendi_giocatori(&bandierine);
	// pronti, attenti
	via(&bandierine);
	printf("G: Il Vincitore è %d\n", risultato_gioco(&bandierine));
	pthread_exit(0);
}

int main(){
	pthread_t t_giocatore1, t_giocatore2, t_giudice;
	pthread_attr_t p_attr;

	pthread_attr_init(&p_attr);

	init_bandierine(&bandierine);

	pthread_create(&t_giocatore1, &p_attr, giocatore, (void *) 1);
	pthread_create(&t_giocatore2, &p_attr, giocatore, (void *) 2);
	pthread_create(&t_giudice, &p_attr, giudice, NULL);

	pthread_join(t_giocatore1, NULL);
	pthread_join(t_giocatore2, NULL);
	pthread_join(t_giudice, NULL);

	pthread_attr_destroy(&p_attr);

	return 0;
}