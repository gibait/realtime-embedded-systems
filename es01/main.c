//
// Created by giba on 27/03/23.
//

/*
  Siano A, B, C, D ed E le procedure che un insieme di processi P1, P2, ..., PN possono invocare e che
devono essere eseguite rispettando i seguenti vincoli di sincronizzazione:
Sono possibili solo due sequenze di esecuzioni delle procedure, sequenze tra loro mutuamente esclusive:
- la prima sequenza prevede che venga eseguita per prima la procedura A. a cui puo' seguire
esclusivamente líesecuzione di una o piu' attivazioni concorrenti della procedura B;
- la seconda sequenza e' costituita dallíesecuzione della procedura C a cui puo' seguire
esclusivamente l'esecuzione della procedura D, o in alternativa a D della procedura E.
Una volta terminata una delle due sequenze una nuova sequenza puo' essere di nuovo iniziata.
utilizzando il meccanismo dei semafori, realizzare le funzioni StartA, EndA, StartB, EndB, .... , StartE,
EndE che, invocate dai processi clienti P1, P2, ..., PN rispettivamente prima e dopo le corrispondenti
procedure, garantiscano il rispetto dei precedenti vincoli. Nel risolvere il problema non e' richiesta la
soluzione ad eventuali problemi di starvation
 */

/*
 * A --> B
 *       B --> B
 *       B --> C
 * C --> D --> A
 *         --> C
 * C --> E --> A
 *         --> C
*/

#include <semaphore.h>

/* Possibili stati in cui si puo' trovare la risorsa */
#define SEQUENZA_NESSUNO 0
#define SEQUENZA_A 1
#define SEQUENZA_B 2
#define SEQUENZA_C 3
#define SEQUENZA_D 4
#define SEQUENZA_E 5
#define SEQUENZA_D_o_E 6

struct gestore_t {
	sem_t mutex;

	// semafori per le procedure
	sem_t sa, sb, sc, sd, se;
	// contatori per le procedure bloccate
	int ca, cb, cc, cd, ce;

	/* stato del sistema */

	// numero di procedure B in esecuzione
	int nb;
	// macchina a stati finiti del sistema
	int stato;
};

void gestore_init(struct gestore_t *g)
{
	sem_init(&g->mutex, 0, 1);
	sem_init(&g->sa, 0, 0);
	sem_init(&g->sb, 0, 0);
	sem_init(&g->sc, 0, 0);
	sem_init(&g->sd, 0, 0);
	sem_init(&g->se, 0, 0);

	g->ca = g->cb = g->cc = g->cd = g->ce = 0;
	g->nb = 0;

	g->stato = SEQUENZA_NESSUNO;
}

void start_a(struct gestore_t *g)
{
	sem_wait(&g->mutex);
	if (g->stato == SEQUENZA_NESSUNO) {
		// nessuna sequenza in esecuzione
		g->stato = SEQUENZA_A;
		// PREVENTIVE POST
		sem_post(&g->sa);
	} else {
		// mi bloccherò quindi incremento il contatore
		g->ca++;
	}
	sem_wait(&g->mutex);
	sem_wait(&g->sa);
}

void end_a(struct gestore_t *g)
{
	sem_wait(&g->mutex);
	// può andare solo una procedura B
	g->stato = SEQUENZA_B;
	// finché c'è una procedura B bloccata la risveglio
	while (g->cb > 0) {
		// mi occupo di aggiornare i contatori
		g->cb--;
		g->nb++;
		sem_post(&g->sb);
	}
	sem_post(&g->mutex);
}

void start_b(struct gestore_t *g)
{
	sem_wait(&g->mutex);
	if (g->stato == SEQUENZA_B) {
		// devo contare le procedure B attive
		g->nb++;
		sem_post(&g->sb);
	} else {
		g->cb++;
	}
	sem_post(&g->mutex);
	sem_wait(&g->sb);
}

void sveglia_A_o_C(struct gestore_t *g)
{
	// se ci sono A bloccati
	if (g->ca) {
		g->stato = SEQUENZA_A;
		g->ca--;
		sem_post(&g->sa);
		// se ci sono C bloccati
	} else if (g->cc) {
		g->stato = SEQUENZA_C;
		g->cc--;
		sem_post(&g->sc);
	} else
		// altrimenti si torna allo stato iniziale
		g->stato = SEQUENZA_NESSUNO;
}

void end_b(struct gestore_t *g)
{
	sem_wait(&g->mutex);

	// decrementa il contatore di B attivi
	g->nb--;

	// se sono l'ultimo B attivo
	if (g->nb > 0) {
		// decido chi risvegliare
		sveglia_A_o_C(g);
	}

	sem_post(&g->mutex);
}

void start_c(struct gestore_t *g)
{
	sem_wait(&g->mutex);

	// caso in cui si parta dallo stato iniziale
	if (g->stato == SEQUENZA_NESSUNO) {
		g->stato = SEQUENZA_C;
		sem_post(&g->sc);
	} else
		g->cc++;

	sem_post(&g->mutex);
	// può essere risvegliato sia da B che da solo
	sem_wait(&g->sc);
}

void end_c(struct gestore_t *g)
{
	sem_wait(&g->mutex);

	// se c'è un D bloccato
	if (g->cd) {
		g->cd--;
		g->stato = SEQUENZA_D;
		sem_post(&g->sd);
		// se c'è un E bloccato
	} else if (g->ce) {
		g->ce--;
		g->stato = SEQUENZA_E;
		sem_post(&g->se);
		// se non ci sono ne D ne E devo salvare lo stato
	} else {
		g->stato = SEQUENZA_D_o_E;
	}

	sem_post(&g->mutex);
}

void start_d(struct gestore_t *g)
{
	sem_wait(&g->mutex);

	// solo se arrivo dopo che il processo C non ha trovato candidati da risvegliare
	if (g->stato == SEQUENZA_D_o_E) {
		g->stato = SEQUENZA_D;
		sem_post(&g->sd);
	} else {
		// tengo traccia del fatto che c'è un D bloccato
		g->cd++;
	}

	sem_post(&g->mutex);
	sem_wait(&g->sd);
}

void end_d(struct gestore_t *g)
{
	sem_wait(&g->mutex);

	sveglia_A_o_C(g);

	sem_post(&g->mutex);
}

void start_e(struct gestore_t *g)
{
	sem_wait(&g->mutex);

	// solo se arrivo dopo che il processo C non ha trovato candidati da risvegliare
	if (g->stato == SEQUENZA_D_o_E) {
		g->stato = SEQUENZA_E;
		sem_post(&g->se);
	} else {
		// tengo traccia che c'è un E bloccato
		g->ce++;
	}

	sem_post(&g->mutex);
	sem_wait(&g->se);
}

void end_e(struct gestore_t *g)
{
	sem_wait(&g->mutex);

	sveglia_A_o_C(g);

	sem_post(&g->mutex);
}
