#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Movie.h"
#include "Rating.h"
#include "CSVReader.h"
#include <pthread.h>
#include <sys/time.h>

#define AUXILIAR_NUMBER_OF_MOVIES 20000
#define AUXILIAR_NUMBER_OF_RATINGS 500000
#define N 100
#define MAXCHAR 1024
#define B 10

int comptador, w, r;
pthread_mutex_t mux;
pthread_cond_t cua_cons, cua_prod;
struct cell *cell_buffer[B];

struct Rating getRating(char* line) {
    struct Rating rating;
    int customerId = atoi(strtok_r(line, ",", &line));
    int ratingValue = atoi(strtok_r(line, ",", &line));
    rating.customerId = customerId;
    rating.rating = ratingValue;
    return rating;
}

void addRatingsToMovie(struct Movie *movie, struct Rating *ratingsToAdd, int numberOfRatings) {
    (*movie).ratings = malloc(numberOfRatings * sizeof(struct Rating));
    (*movie).numberOfRatings = numberOfRatings;
    for(int i=0; i < numberOfRatings; i++) {
      (*movie).ratings[i].customerId = ratingsToAdd[i].customerId;
      (*movie).ratings[i].rating = ratingsToAdd[i].rating;
    }
}

void addMovieToCurrentMovies(struct Movie *movies, struct Movie *movieToAdd, int indexToAdd) {
    movies[indexToAdd].movieId = (*movieToAdd).movieId;
    movies[indexToAdd].ratings = (*movieToAdd).ratings;
    movies[indexToAdd].numberOfRatings = (*movieToAdd).numberOfRatings;
    (*movieToAdd).movieId = -1;
    (*movieToAdd).ratings = NULL;
}

int lineIsMovie(char *line) {
    int hasComma = 0;
    int idx = 0;
    int lineSize = strlen(line);
    while (!hasComma && idx < lineSize) {
        if(line[idx] == ',')
            hasComma = 1;
        idx++;
    }
    return 1 - hasComma;
}

int getMovieId(char *line) {
    int movieId = atoi(strtok_r(line, ":", &line));
    return movieId;
}

struct cell {
    int numberLines;
    int movieId[N];
    char line[N][MAXCHAR];
};


void *consumidor(void *arg) {
    struct Movie *currentMovies = malloc(AUXILIAR_NUMBER_OF_MOVIES*sizeof(struct Movie));
    int currentIndexOfMovies = 0; // It coincides with the number of movies available.
    struct Movie currentMovie;
    struct Rating *currentRatings = malloc(AUXILIAR_NUMBER_OF_RATINGS*sizeof(struct Rating));
    int currentIndexOfRatings = 0; // It coincides with the number of ratings available per movie when reading.
    struct Rating currentRating;

    struct FileMovies *moviesToReturn;
    moviesToReturn = malloc(sizeof(struct FileMovies));
    struct cell *cell_cons, *tmp;
    cell_cons = malloc(sizeof(struct cell));
    int prev_movie = 1;

    int cons_acabat = 0;
    // Consumidor
    while (!cons_acabat) {
        // Zona critica
        pthread_mutex_lock(&mux);
        // Si el buffer está "vacio", consumidor espera
        if (comptador == 0) {
            pthread_cond_wait(&cua_cons, &mux);
        }
        // Intercanvi de punters
        tmp = cell_buffer[r];
        cell_buffer[r] = cell_cons;
        cell_cons = tmp;
        r = (r + 1) % B;
        comptador--;
        pthread_cond_signal(&cua_prod);
        pthread_mutex_unlock(&mux);

        // Condició per sortir del bucle
        if (cell_cons->numberLines < N){
            cons_acabat = 1;
        }

        int j, cur_movie;
        for (j = 0; j < cell_cons->numberLines ; j++) {
            cur_movie = cell_cons->movieId[j];
            if (cur_movie != prev_movie) {
                currentMovie.movieId = prev_movie;
                addRatingsToMovie(&currentMovie, currentRatings, currentIndexOfRatings);
                addMovieToCurrentMovies(currentMovies, &currentMovie, currentIndexOfMovies); //Put ratings too
                currentIndexOfMovies++;
                currentIndexOfRatings = 0; // canviem de pelicula
                prev_movie = cur_movie;
            }
            currentRating = getRating(cell_cons->line[j]);
            currentRatings[currentIndexOfRatings].customerId = currentRating.customerId;
            currentRatings[currentIndexOfRatings].rating = currentRating.rating;
            currentIndexOfRatings++;
        }
    }
    printf("[C] Processed last movie: %d\n", prev_movie);
    currentMovie.movieId = prev_movie + 1;
    // When we finish iterating we have one last movie to save.
    addRatingsToMovie(&currentMovie, currentRatings, currentIndexOfRatings);
    addMovieToCurrentMovies(currentMovies, &currentMovie, currentIndexOfMovies);
    currentIndexOfMovies++;
    // currentIndexOfMovies is the number of movies
    currentMovies = (struct Movie*) realloc(currentMovies, currentIndexOfMovies*sizeof(struct Movie));
    if(currentMovies == NULL) {
        printf("Memory management failed. Contact professors\n");
        exit(1);
    }
    moviesToReturn->movies = currentMovies;
    moviesToReturn->numberOfMovies = currentIndexOfMovies;
    // We only free currentRatings as we keep using currentMovies as returnable variable
    free(currentRatings);
    free(cell_cons);
    return ((void *) moviesToReturn);
}

struct FileMovies getMoviesFromFile(FILE* trainingData) {
    int movieId = -1;
    struct cell *cell_producer, *tmp;
    cell_producer = malloc(sizeof(struct cell));
    int j;
    for (j=0; j<B; j++) {
        cell_buffer[j] =  malloc(sizeof(struct cell));
    }
    struct FileMovies *tret; // Structure of return
    struct FileMovies moviesToReturn; // Structure of return

    // Pthread
    comptador = 0;
    pthread_t cons;
    pthread_mutex_init(&mux, NULL); 
    pthread_cond_init(&cua_cons, NULL);
    pthread_cond_init(&cua_prod, NULL);
    int err;
    err = pthread_create(&cons, NULL, consumidor, NULL);
    
    int acabat = 0;
    // Productor
    while (!acabat){
        // Generar cellas amb N' elements
        int i = 0;
        while (i < N && !acabat) {
            // Si acabem de llegir el fitxer, 
            if (!fgets(cell_producer->line[i], MAXCHAR, trainingData)) { // Tip to copy directly to cella 1:
                acabat = 1;
                printf("[P] Finished file\n");
            }
            else {
                // Si la linea es la id del movie, començar amb una nova pelicula
                if (lineIsMovie(cell_producer->line[i])) {
                    movieId = getMovieId(cell_producer->line[i]);
                }
                else {
                    // Emmagatzmar cella movieId + line
                    cell_producer->movieId[i] = movieId;
                    i++;
                }
            }
        }
        cell_producer->numberLines = i;
        // Zona critica
        pthread_mutex_lock(&mux);
        // Si el buffer está "lleno", productor espera
        if (comptador == B) {
            pthread_cond_wait(&cua_prod , &mux);
        }
        // Intercambio de punteros productor y buffer
        tmp = cell_buffer[w];
        cell_buffer[w] = cell_producer;
        cell_producer = tmp;
        w = (w + 1) % B;
        comptador++;
        pthread_cond_signal(&cua_cons);
        pthread_mutex_unlock(&mux);
    }
    err = pthread_join(cons, (void *) &tret);
    moviesToReturn.movies = (struct Movie*) tret->movies;
    moviesToReturn.numberOfMovies = (int) tret->numberOfMovies;
    free(cell_producer);
    for (j=0; j<B; j++) {
        free(cell_buffer[j]);
    }
    free(tret);
    return moviesToReturn;
}

struct FileMovies getMoviesFromCSV(const char  *filepath) {
    FILE* trainingData = fopen(filepath, "r");
    struct FileMovies moviesToReturn;
    if (!trainingData) 
    {
        printf("Failed to open text file, does the file exist?\n");
        exit(1);
    }
    // Chronological measurement of getMoviesFromFile()
    struct timeval tv1, tv2; // Cronologic
    clock_t t1, t2;
    gettimeofday(&tv1, NULL);
    t1 = clock();

    moviesToReturn = getMoviesFromFile(trainingData);

    gettimeofday(&tv2, NULL);
    t2 = clock();

    printf("--- Time measurement of getMoviesFromFile() ---\n");
    printf("Temps de CPU : %f seconds\n", 
            (double)(t2 - t1) / (double) CLOCKS_PER_SEC);
    printf("Temps cronologic: %f seconds\n",
            (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
            (double) (tv2.tv_sec - tv1.tv_sec));
    fclose(trainingData);
    return moviesToReturn;
}
