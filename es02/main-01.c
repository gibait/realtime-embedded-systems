//
// Created by giba on 11/04/23.
//

#include <semaphore.h>

struct gestore_t {
	sem_t mutex;

	sem_t s_ab, s_r;

	// active counters
	int n_ab, n_r;
	// blocked counters
	int b_ab, b_r;

} gestore;

void start_proc_a_or_b() {
	sem_wait(&gestore.mutex);

	// se c'è un proc RESET attivo mi blocco
	if (gestore.n_r) {
		gestore.b_ab++;
	} else {
		// altrimenti preemptive post
		sem_post(&gestore.s_ab);
		gestore.n_ab++;
	}

	sem_post(&gestore.mutex);
	sem_wait(&gestore.s_ab);

}

void end_proc_a_or_b() {
	sem_wait(&gestore.mutex);

	gestore.n_ab--;

	// se non c'è nessun altro proc A or B attivo
	if (gestore.n_ab == 0) {
		// diamo priorità ai proc RESET bloccati
		if (gestore.b_r) {
			gestore.b_r--;
			gestore.n_r++;
			sem_post(&gestore.s_r);

		} else if (gestore.b_ab > 0) {
		// altrimenti mandiamo un proc A or B
			gestore.b_ab--;
			gestore.n_ab++;
			sem_post(&gestore.s_ab);
		}
	}

	sem_post(&gestore.mutex);
}

void start_proc_r() {
	sem_wait(&gestore.mutex);

	// se c'è un proc A or B attivo mi blocco
	// supponiamo un solo processo R
	if (gestore.n_ab > 0) {
		gestore.b_r++;
	} else {
	// altrimenti preemptive post
		sem_post(&gestore.s_r);
		gestore.n_r++;
	}

	sem_post(&gestore.mutex);
	sem_post(&gestore.s_r);
}

void end_proc_r() {
	sem_wait(&gestore.mutex);

	// diamo priorità ai processi RESET
	if (gestore.b_r) {
		gestore.b_r--;
		gestore.n_r++;
		sem_post(&gestore.s_r);
	// altrimenti risvegliamo un proc A or B
	} else if (gestore.b_ab) {
		gestore.b_ab--;
		gestore.n_ab++;
		sem_post(&gestore.s_ab);
	}

	sem_post(&gestore.mutex);
}