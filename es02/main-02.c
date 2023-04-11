//
// Created by giba on 11/04/23.
//

#include <semaphore.h>

struct gestore_t {
	sem_t mutex;

	sem_t s_a, s_b, s_r;

	// active counters
	int n_a, n_b, n_r;
	// blocked counters
	int b_a, b_b, b_r;

} gestore;

void gestore_init(struct gestore_t* gestore) {
	sem_init(&gestore->mutex, 0, 1);

	sem_init(&gestore->s_a, 0, 1);
	sem_init(&gestore->s_b, 0, 0);
	sem_init(&gestore->s_r, 0, 0);

	gestore->n_a = gestore->n_b = gestore->n_r = 0;
	gestore->b_a = gestore->b_b = gestore->b_r = 0;
}

void wake_up_someone() {

	if (gestore.n_a == 0 && gestore.n_b == 0) {
		if (gestore.b_r > 0) {
			gestore.b_r--;
			gestore.n_r++;
			sem_post(&gestore.s_r);
		} else if (gestore.b_a > 0) {
			gestore.b_a--;
			gestore.n_a++;
			sem_post(&gestore.s_a);
		} else if (gestore.b_b > 0) {
			gestore.b_b--;
			gestore.n_b++;
			sem_post(&gestore.s_b);
		}
	}

}

void start_proc_a() {
	sem_wait(&gestore.mutex);

	if (gestore.n_a > 0 && gestore.n_r > 0) {
		gestore.b_a++;
	} else {
		gestore.n_a++;
		sem_post(&gestore.s_a);
	}

	sem_post(&gestore.mutex);
	sem_wait(&gestore.s_a);
}

void end_proc_a() {
	sem_wait(&gestore.mutex);

	gestore.n_a--;

	wake_up_someone();

	sem_post(&gestore.mutex);
}

void start_proc_b() {
	sem_wait(&gestore.mutex);

	if (gestore.n_b > 0 && gestore.n_r > 0) {
		gestore.b_b++;
	} else {
		gestore.n_b++;
		sem_post(&gestore.s_b);
	}

	sem_post(&gestore.mutex);
	sem_wait(&gestore.s_b);
}

void end_proc_b() {
	sem_wait(&gestore.mutex);

	gestore.n_b--;

	wake_up_someone();

	sem_post(&gestore.mutex);
}

void start_proc_r() {
	sem_wait(&gestore.mutex);

	// se c'è un proc A or B attivo mi blocco
	// supponiamo un solo processo R
	if (gestore.n_a > 0 || gestore.n_b > 0) {
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

	wake_up_someone();

	sem_post(&gestore.mutex);
}
