#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_FILS  2 
#define NUM_RECTS 1E09

double valor_pi;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void integral(int id)
{
    int i;
    double el_meu_resultat;
    double mid, height, width;

    el_meu_resultat = 0.0;

    width = 1.0 / (double) NUM_RECTS;
    for(i = id; i < NUM_RECTS; i += NUM_FILS) {
        mid = (i + 0.5) * width;
        height = 4.0 / (1.0 + mid * mid);
        el_meu_resultat += height * width;
    }

    pthread_mutex_lock(&mutex); // lock
    valor_pi += el_meu_resultat; 
    pthread_mutex_unlock(&mutex);// unlock
}


void *thread_fn(void *arg)
{
    int i = (long int) arg;
    printf("Soc el fil numero %d\n", i);
    integral(i);

    return ((void *)0);
}

int main(void)
{
    pthread_t ntid[NUM_FILS];
    long int i;

    valor_pi = 0.0;

    for(i = 0; i < NUM_FILS; i++)
        pthread_create(&(ntid[i]), NULL, thread_fn, (void *) i);

    for(i = 0; i < NUM_FILS; i++)
        pthread_join(ntid[i], NULL);

    printf("Valor de pi: %e\n", valor_pi);
}
