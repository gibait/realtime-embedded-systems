//
// Created by giba on 14/04/23.
//

#include <semaphore.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define RUNNERS 2
#define NONE -1

typedef enum {false, true} bool;

struct bandierine_t {
	sem_t mutex;

	sem_t pronti, via, fine;

	int chi_ha_preso_la_bandiera;
	int vincitore;
	bool salvo;

} bandierine;

void init_bandierine(struct bandierine_t *b)
{
	sem_init(&b->mutex, 0, 1);
	sem_init(&b->pronti, 0, 0);
	sem_init(&b->via, 0, 0);
	sem_init(&b->fine, 0, 0);

	b->chi_ha_preso_la_bandiera = b->vincitore = NONE;
	b->salvo = false;
}

void microPause(void) {
	struct timespec t;
	t.tv_sec = 0;
	t.tv_nsec = (rand() % + 1) * 1000000;
	nanosleep(&t, NULL);
}

/*
    Da il via al gioco sbloccando i giocatori in attesa
*/
void via(struct bandierine_t *b)
{
	for (int i = 0; i < RUNNERS; i++) {
		sem_post(&b->via);
	}
}

/*
    Blocca i giocatori che attendono il via
*/
void attendi_il_via(struct bandierine_t *b, int n)
{
	sem_post(&b->pronti);
	sem_wait(&b->via);
}

/*
    Prende la bandierina (se disponibile).
        1 -> Se il Giocatore prende la bandierina
        0 -> Se il Giocatore NON prende la bandierina
*/
int bandierina_presa(struct bandierine_t *b, int n){
	int retr = 0;

	sem_wait(&b->mutex);

	if (b->chi_ha_preso_la_bandiera == NONE) {
		b->chi_ha_preso_la_bandiera = n;
		retr = 1;
	}

	sem_post(&b->mutex);
	return retr;
}

/*
    1 -> Se il Giocatore NON è stato preso
    0 -> Se il Giocatore è stato preso
*/
int sono_salvo(struct bandierine_t *b, int n)
{
	int retr = 0;

	sem_wait(&b->mutex);

	if (b->salvo == false) {
		b->salvo = true;
		b->vincitore = n;
		retr = 1;
	}

	sem_post(&b->fine);
	sem_post(&b->mutex);

	return retr;
}

/*
    1 -> Se il Giocatore NON si è ancora salvato
    0 -> Se il Giocatore si è già salvato
*/
int ti_ho_preso(struct bandierine_t *b, int n)
{
	int retr = 0;

	sem_wait(&b->mutex);

	if (b->salvo == false) {
		b->salvo = true;
		b->vincitore = n;
		retr = 1;
	}

	sem_post(&b->fine);
	sem_post(&b->mutex);

	return retr;
}

/*
    Ritorna il Numero del Vincitore
*/
int risultato_gioco(struct bandierine_t *b){
	int retr;

	sem_wait(&b->fine);

	sem_wait(&b->mutex);
	retr = b->vincitore;
	sem_post(&b->mutex);

	return retr;
}

/*
    Attende che entrambi i giocatori siano pronti in attesa prima di dare il via
*/
void attendi_giocatori(struct bandierine_t *b)
{
	sem_wait(&b->pronti);
	sem_wait(&b->pronti);
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