#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Movie.h"
#include "Rating.h"
#include "CSVReader.h"

#define AUXILIAR_NUMBER_OF_MOVIES 20000
#define AUXILIAR_NUMBER_OF_RATINGS 500000

/*
   struct Rating getRating(char* line) {
   struct Rating rating;
   int customerId = atoi(strtok(line, ","));
   int ratingValue = atoi(strtok(NULL, ","));
   rating.customerId = customerId;
   rating.rating = ratingValue;
   return rating;
   }*/

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
/*
   int getMovieId(char *line) {
   return atoi(strtok(line, ":"));
   }
 */

/*** CHANGES BEGIN HERE ***/

/* S'evita utilitzar strtok ja que no es thread-safe. */

struct Rating getRating(char* line) {
    struct Rating rating;
    int customerId = atoi(strtok_r(line, ",", &line));
    int ratingValue = atoi(strtok_r(line, ",", &line));
    rating.customerId = customerId;
    rating.rating = ratingValue;
    return rating;
}

int getMovieId(char *line) {
    int movieId = atoi(strtok_r(line, ":", &line));
    return movieId;
}

#include <pthread.h>
#include <sys/time.h>

#define CELL_NELEMS 1000
#define MAXCHAR 100

struct cell {
    int nelems; /* Nombre elements que contenen dades */
    int movieId[CELL_NELEMS]; /* Identificador de la peli */
    char line[CELL_NELEMS][MAXCHAR]; /* Linies llegides del fitxer */
};

struct buffer {
    /* Només un element al buffer. */
    int buit;
    struct cell *cell_buffer;
};

struct thread_args {
    struct buffer *buffer_prodcons;
};

/* Lock para el algoritmo del productor consumidor step1 i step2 */
pthread_mutex_t global_mutex_prod_cons = PTHREAD_MUTEX_INITIALIZER;

/* Variables condicionals necessaries per implementar l'algorisme del
 * productor-consumidor */
pthread_cond_t producer = PTHREAD_COND_INITIALIZER;
pthread_cond_t consumer = PTHREAD_COND_INITIALIZER;

/* This is the the code for consumer. We get the
   lines that have been read from producer and extract data */ 

void *getMoviesFromFilesConsumer(void *arg)
{
    int index, nelems, finished;

    /* Arguments que ens han passat */
    struct thread_args *arguments = (struct thread_args *) arg;
    struct buffer *buffer_prodcons = arguments->buffer_prodcons; 

    /* Cell consumer */
    struct cell *cell_consumer, *cell_tmp;

    /* Reservem memoria per a cell_consumer */
    cell_consumer = malloc(sizeof(struct cell));

    /* Codi per analitzar les pel-licules que s'extreuen. Utilitzem
       les mateixes variables que al codi original. */
    int movieId  = - 1;
    int currentIndexOfMovies = 0;
    struct Movie currentMovie; /* Pel-licula analitzada actualment */
    struct Movie *currentMovies = malloc(AUXILIAR_NUMBER_OF_MOVIES*sizeof(struct Movie));

    int currentIndexOfRatings = 0; 
    struct Rating currentRating;
    struct Rating *currentRatings = malloc(AUXILIAR_NUMBER_OF_RATINGS*sizeof(struct Rating));

    /* Estructura que retornem des del consumidor al productor */
    struct FileMovies *moviesToReturn = malloc(sizeof(struct FileMovies));

    /* Comencem */
    finished = 0; /* Serveix per saber si hem acabat de processar les dades */

    while (!finished) {
        /* Intercanvi de les dades amb el productor. Nomes hi ha un
           element al buffer, cosa que simplica l'intercanvi. El codi
           es a la fitxa 2 penjada al campus. */
        pthread_mutex_lock(&global_mutex_prod_cons);
        if (buffer_prodcons->buit)
            pthread_cond_wait(&consumer, &global_mutex_prod_cons);

        /* Here is the trick with the pointers */
        cell_tmp = buffer_prodcons->cell_buffer;
        buffer_prodcons->cell_buffer = cell_consumer;
        cell_consumer = cell_tmp;

        buffer_prodcons->buit = 1; 

        pthread_cond_signal(&producer);
        pthread_mutex_unlock(&global_mutex_prod_cons);

        /* Extraiem les dades del buffer que hem rebut */
        index = 0;
        nelems = cell_consumer->nelems; 
        while (index < nelems) {
            if (cell_consumer->movieId[index] != movieId) {
                if (movieId == -1)
                    movieId = cell_consumer->movieId[index];
                else {
                    addRatingsToMovie(&currentMovie, currentRatings, currentIndexOfRatings);
                    addMovieToCurrentMovies(currentMovies, &currentMovie, currentIndexOfMovies); //Put ratings too
                    currentIndexOfMovies++;
                    currentIndexOfRatings = 0; 
                    movieId = cell_consumer->movieId[index];
                }

                currentMovie.movieId = movieId;
            }

            currentRating = getRating(cell_consumer->line[index]);

            currentRatings[currentIndexOfRatings].customerId = currentRating.customerId;
            currentRatings[currentIndexOfRatings].rating = currentRating.rating;
            currentIndexOfRatings++;

            index++;
        }

        if (index < CELL_NELEMS)
            finished = 1;
    }

    /* Fem que el fa el codi original. The last movie to save */

    if (index != 0) {  
      addRatingsToMovie(&currentMovie, currentRatings, currentIndexOfRatings);
      addMovieToCurrentMovies(currentMovies, &currentMovie, currentIndexOfMovies); //Put ratings too
      currentIndexOfMovies++;
    }

    /* Fem el que fa el codi original. Adaptem la mida del vector a la que hem obtingut */
    currentMovies = (struct Movie*) realloc(currentMovies, currentIndexOfMovies*sizeof(struct Movie));
    if(currentMovies == NULL) {
        printf("Memory management failed. Contact professors\n");
        exit(1);
    }
    moviesToReturn->movies = currentMovies;
    moviesToReturn->numberOfMovies = currentIndexOfMovies;

    /* Alliberem memoria */
    free(cell_consumer);
    free(currentRatings);

    return ((void *) moviesToReturn);
}


struct FileMovies *getMoviesFromFileProducer(FILE *trainingData)
{
    int currentMovieId, index, finished;
    char line[MAXCHAR];

    struct FilesMovies *moviesToReturn;

    struct buffer *buffer_prodcons;
    struct cell *cell_producer, *cell_tmp;

    /* Reservem memoria per a cell_producer */
    cell_producer = malloc(sizeof(struct cell));

    /* Iniciem el buffer step1 */
    buffer_prodcons = malloc(sizeof(struct buffer));
    buffer_prodcons->buit = 1; /* La cel-la no te dades */
    buffer_prodcons->cell_buffer = malloc(sizeof(struct cell));

    /* Fil secundari */
    struct thread_args thread_args;
    pthread_t thread_id;

    /* Creem el fil */
    thread_args.buffer_prodcons = buffer_prodcons;
    pthread_create(&thread_id, NULL,getMoviesFromFilesConsumer, (void*) &thread_args); 

    /* Comencem */
    finished = 0; /* Serveix per saber si hem acabat de llegir les dades del fitxer */

    /* Se suposa que la primera linia es un ID de peli */
    fgets(line, sizeof(line), trainingData);
    if (lineIsMovie(line)) {
        currentMovieId = getMovieId(line);
    } else {
        printf("ERROR: getMoviesFromFileProducer: error, first line is not a movieId\n");
        exit(1);
    }

    /* Ara ja tenim la ID de la peli. Emmagatzemarem les dades directament a la cel-la per evitar
       fer servir strcpy */
    while (!finished) {
        /* Llegim un block de CELL_NELEMS linies */
        index = 0;
        while (index < CELL_NELEMS) {
            if (fgets(cell_producer->line[index], MAXCHAR, trainingData)) {
                if (lineIsMovie(cell_producer->line[index])) {
                    currentMovieId = getMovieId(cell_producer->line[index]);
                    //printf("%d\n", currentMovieId);
                } else {
                    cell_producer->movieId[index] = currentMovieId;
                    index++;
                }
            } else {
                /* Indiquem que hem acabat */
                finished = 1;
                break;
            }
        }
        cell_producer->nelems = index;

        /* Intercanvi de les dades amb el consumidor step 1. Nomes hi ha un
           element al buffer, cosa que simplica l'intercanvi. El codi
           es a la fitxa 2 penjada al campus. */
        pthread_mutex_lock(&global_mutex_prod_cons);
        if (!buffer_prodcons->buit) /* Amb un 'if' ja en tenim prou */
            pthread_cond_wait(&producer, &global_mutex_prod_cons);

        /* Here is the trick with the pointers!!! */
        cell_tmp = buffer_prodcons->cell_buffer;
        buffer_prodcons->cell_buffer = cell_producer;
        cell_producer = cell_tmp;

        /* La cel-la te dades */
        buffer_prodcons->buit = 0; 

        pthread_cond_signal(&consumer);
        pthread_mutex_unlock(&global_mutex_prod_cons);

        /* Hem acabat quan ja no hi ha mes dades a llegir del fitxer */
        if (index < CELL_NELEMS)
            finished = 1;
    }

    /* Esperem que acabi el consumidor 1 */
    pthread_join(thread_id, (void *) &moviesToReturn);

    free(cell_producer);
    free(buffer_prodcons->cell_buffer);
    free(buffer_prodcons);

    return (void *) moviesToReturn;
}


struct FileMovies getMoviesFromFile(FILE *trainingData)
{
    struct timeval tv1, tv2; // Cronologic
    clock_t t1, t2;

    struct FileMovies *moviesProducer, moviesToReturn;

    gettimeofday(&tv1, NULL);
    t1 = clock();

    moviesProducer = (struct FileMovies *) getMoviesFromFileProducer(trainingData);

    /* Copiem tots els membres de l'estructura */
    memcpy(&moviesToReturn, moviesProducer, sizeof(struct FileMovies));

    /* Alliberem l'estructura, no pas el contingut */
    free(moviesProducer); 

    gettimeofday(&tv2, NULL);
    t2 = clock();

    printf("Analisi temps execucio getMoviesFromFile\n");
    printf("Temps de CPU: %f seconds\n", 
            (double)(t2 - t1) / (double) CLOCKS_PER_SEC);
    printf("Temps cronologic: %f seconds\n",
            (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
            (double) (tv2.tv_sec - tv1.tv_sec));

    /* Hem acabat! */
    return moviesToReturn;
}

/*
   Codi previ sense fils

   struct FileMovies getMoviesFromFile(FILE* trainingData) {
   char line[MAXCHAR]; 
   int movieId = -1;
   struct Movie *currentMovies = malloc(AUXILIAR_NUMBER_OF_MOVIES*sizeof(struct Movie));
   int currentIndexOfMovies = 0; // It coincides with the number of movies available.
   struct Movie currentMovie;
   struct Rating *currentRatings = malloc(AUXILIAR_NUMBER_OF_RATINGS*sizeof(struct Rating));
   int currentIndexOfRatings = 0; // It coincides with the number of ratings available per movie when reading.
   struct Rating currentRating;
   struct FileMovies moviesToReturn;

   while(fgets(line, sizeof(line), trainingData)) {
   if(lineIsMovie(line)) {
   if(movieId != -1) {
   printf("%d %d\n", currentIndexOfMovies, currentIndexOfRatings);
   addRatingsToMovie(&currentMovie, currentRatings, currentIndexOfRatings);
   addMovieToCurrentMovies(currentMovies, &currentMovie, currentIndexOfMovies); //Put ratings too
   currentIndexOfMovies++;
   currentIndexOfRatings = 0; 
   }
   movieId = getMovieId(line);
   currentMovie.movieId = movieId;
   }
   else {
   currentRating = getRating(line);
   currentRatings[currentIndexOfRatings].customerId = currentRating.customerId;
   currentRatings[currentIndexOfRatings].rating = currentRating.rating;
   currentIndexOfRatings++;
   }
   }
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
moviesToReturn.movies = currentMovies;
moviesToReturn.numberOfMovies = currentIndexOfMovies;
// We only free currentRatings as we keep using currentMovies as returnable variable
free(currentRatings);
return moviesToReturn;
}
 */

/*** CHANGES END HERE ***/


struct FileMovies getMoviesFromCSV(const char  *filepath) {
    FILE* trainingData = fopen(filepath, "r");
    struct FileMovies moviesToReturn;
    if (!trainingData) 
    {
        printf("Failed to open text file, does the file exist?\n");
        exit(1);
    }
    moviesToReturn = getMoviesFromFile(trainingData);
    fclose(trainingData);
    return moviesToReturn;
}
