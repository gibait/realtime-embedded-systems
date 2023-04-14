//
// Created by giba on 11/04/23.
//

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <semaphore.h>

#define my_printf(x, ...) printf(x, ##__VA_ARGS__); fflush(stdout)
#define do_something() usleep(rand() % 1000 + 1000)

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

void procA(int thread_idx) {
	start_proc_a();

	my_printf("<%dA", thread_idx);
	do_something();
	my_printf("<%dA", thread_idx);

	end_proc_a();
}

void procB(int thread_idx) {
	start_proc_b();

	my_printf("<%dB", thread_idx);
	do_something();
	my_printf("<%dB", thread_idx);

	end_proc_b();
}

void Reset(int thread_idx) {
	start_proc_r();

	my_printf("<%dR", thread_idx);
	do_something();
	my_printf("<%dR", thread_idx);

	end_proc_r();
}

void* thread_body(void* arg)
{
	int thread_idx = *((int*) arg);
	while (1)
	{
		int r = rand();

		my_printf("|%d->%s|", thread_idx, (r%3 ==0  ? "A" : (r%3==1 ? "B" : "R")));

		if (r % 3 == 0) procA(thread_idx);
		else if (r % 3 == 1) procB(thread_idx);
		else if (r % 3 == 2) Reset(thread_idx);
	}
}


int main(int argc, char* argv[])
{
	srand(time(NULL));

	const int k_num_threads = 10;
	pthread_t my_threads[k_num_threads];

	gestore_init(&gestore);

	// stores threads id in an array for debugging
	int thread_ids[k_num_threads];
	for (int i = 0; i < k_num_threads; i++)
		thread_ids[i] = i;

	// creates and starts threads
	for (int i = 0; i < k_num_threads; i++)
	{
		if (pthread_create(&my_threads[i], NULL, thread_body, (void*) &thread_ids[i]) != 0)
		{
			perror("pthread_create() error\n");
			return 1;
		}
	}

	sleep(60);

	return 0;
}