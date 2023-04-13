//
// Created by giba on 11/04/23.
//

#include <pthread.h>

struct mailbox_t {
	pthread_cond_t msg_disponibile, busta_disponibile;
};

struct gestore_t {
	pthread_mutex_t mutex;

	struct mailbox_t mailbox;

	unsigned buste_libere, non_letti
} gestore;


void send(int messaggio)
{
	pthread_mutex_lock(&gestore.mutex);

	while(gestore.buste_libere == 0) {
		pthread_cond_wait(&gestore.mailbox.busta_disponibile, &gestore.mutex);
	}

	gestore.buste_libere--;
	gestore.non_letti++;


	pthread_mutex_unlock(&gestore.mutex);
	pthread_cond_broadcast(&gestore.mailbox.msg_disponibile);
}

int receive()
{
	pthread_mutex_lock(&gestore.mutex);
	
	while (gestore.non_letti == 0) {
		pthread_cond_wait(&gestore.mailbox.msg_disponibile, &gestore.mutex);
	}

	gestore.non_letti--;
	gestore.buste_libere++;

	pthread_mutex_unlock(&gestore.mutex);
	pthread_cond_broadcast(&gestore.mailbox.busta_disponibile);
}