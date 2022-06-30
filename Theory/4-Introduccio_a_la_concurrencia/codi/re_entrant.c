/*
 *  Exemple de codi en el qual el fil principal agafa dos cops la 
 *  mateixa clau. Observar que darrer missatge no s'imprimeix per
 *  pantalla. Es un exemple divulgatiu (ja que no te gaire sentit
 *  agafar una clau si nomes hi ha un fil).
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main ()
{
	printf("Agago per primer cop la clau\n");
	pthread_mutex_lock(&mutex);
	printf("Agafo per segon cop la clau\n");
	pthread_mutex_lock(&mutex);
	printf("He agafat dos cops la mateixa clau\n");

	return  0;
}

