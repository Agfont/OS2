/*
 *  
 *  Exemple de codi dels filosofs en que es pot produir un deadlock. El filosof 0
 *  es troba entre la forquilla 0 i la 1, el filosof 1 entre la forquilla 1 i 2,
 *  i aixi successivament. El filosof 4 es troba entre la forquilla 4 i 0. Els
 *  filosofs comencen per agafar la forquilla de l'esquerra, cosa que pot provocar
 *  un deadlock.
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define NUM_FILOSOFS   5

pthread_mutex_t forquilla[NUM_FILOSOFS];

void *filosof(void *arg)
{
	int filosof;
	int iteracions;
	int forquilla_primera, forquilla_segona;

	filosof = (int) arg;
	forquilla_primera = filosof;
	forquilla_segona    = (filosof + 1);

	if (forquilla_segona == NUM_FILOSOFS)
		forquilla_segona = 0;

	iteracions = 0;
	while (1) {
		printf("Filosof %d: %d\n", filosof, iteracions);
		// Pensa
		usleep(10);
		// Menja
		pthread_mutex_lock(&forquilla[forquilla_primera]);
		pthread_mutex_lock(&forquilla[forquilla_segona]);
		usleep(10);
		iteracions++;
		pthread_mutex_unlock(&forquilla[forquilla_segona]);
		pthread_mutex_unlock(&forquilla[forquilla_primera]);
	}

	return ((void *)0);
}


int main ()
{
	int i;
	pthread_t ntid[NUM_FILOSOFS];

	// Inicialitzem els mutex
	for(i = 0; i < NUM_FILOSOFS; i++)
		pthread_mutex_init(&forquilla[i], NULL);

	// Creem els fils
	for(i = 0; i < NUM_FILOSOFS; i++)
		pthread_create(&(ntid[i]), NULL, filosof, (void *) i);

	// Ens esperem que acabi el filosof 0 (que no acabara mai)
	// Així evitem que el fil principal acabi
	pthread_join(ntid[0], NULL);

	return  0;
}

