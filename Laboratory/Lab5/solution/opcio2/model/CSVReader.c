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

/* S'evita utilitzar strtok ja que no es thread-safe. Codi que s'entrega als alumnes */

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

#define BUFFER_NCELLS 100
#define CELL_NELEMS 10000
#define MAXCHAR 100

/* Step 1 */

struct cell {
    int nelems; /* Nombre elements que contenen dades */
    int movieId[CELL_NELEMS]; /* Identificador de la peli */
    char line[CELL_NELEMS][MAXCHAR]; /* Linies llegides del fitxer */
};

struct buffer {
    /* El buffer te BUFFER_NCELLS elements */
    int write; // Index on es pot escriure
    int read; //  Index d'on s'ha de llegir
    int counter; // Nombre d'elements del buffer
    struct cell *cell_buffer[BUFFER_NCELLS];
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

/**
 *
 *  This function tranfers the cell information to the buffer. It is a bit
 *  tricky since it performs the next: it takes the cell that is passed as
 *  argment and introduces it in the buffer, whereas the cell that is in the
 *  buffer is taken so that the producer can use it. The cell that is in the
 *  buffer is in fact a cell that can be overwritten. Rather than copying data
 *  from the producer to the buffer, "take" the unused cell of the buffer and
 *  return it to the producer to that is can be used. To do this we just play
 *  with the pointers.
 *
 */

struct cell *producer_transfer_to_buffer(struct buffer *buffer_prodcons, struct cell *cell_producer)
{
    struct cell *cell_buffer;

    pthread_mutex_lock(&global_mutex_prod_cons);
    while (buffer_prodcons->counter == BUFFER_NCELLS) {
        pthread_cond_wait(&producer, &global_mutex_prod_cons);
    }

    /* Here is the trick with the pointers!!! */
    cell_buffer = buffer_prodcons->cell_buffer[buffer_prodcons->write];
    buffer_prodcons->cell_buffer[buffer_prodcons->write] = cell_producer;

    buffer_prodcons->write++;
    if (buffer_prodcons->write == BUFFER_NCELLS)
        buffer_prodcons->write = 0;

    buffer_prodcons->counter++;

    pthread_cond_signal(&consumer);
    pthread_mutex_unlock(&global_mutex_prod_cons);

    return cell_buffer;
}

/**
 *
 *  This is the function that gets data from the buffer. It is the equivalent
 *  of the function producer_transfer_to_buffer. Here we also play with the
 *  buffers. The consumer gives as argument a cell that contains information
 *  that already has been processed. And it gets a cell that has to be
 *  analyzed. Since we use pointers, we just interchange them. There is no need
 *  to copy data.
 *
 */

struct cell *consumer_get_from_buffer(struct buffer *buffer_prodcons, struct cell *cell_consumer)
{
  struct cell *cell_buffer;

  pthread_mutex_lock(&global_mutex_prod_cons);
  while (buffer_prodcons->counter == 0) {
    pthread_cond_wait(&consumer, &global_mutex_prod_cons);
  }

  /* This condition is satisfied when there are elements in
   * the buffer. Observe that an empy buffer (nelems == 0) is
   * returned to the consumer if the analysis has finished. */ 

  /* Here is the trick with the pointers */
  cell_buffer = buffer_prodcons->cell_buffer[buffer_prodcons->read];
  buffer_prodcons->cell_buffer[buffer_prodcons->read] = cell_consumer;

  buffer_prodcons->read++;
  if (buffer_prodcons->read == BUFFER_NCELLS)
    buffer_prodcons->read = 0;

  buffer_prodcons->counter--;
  pthread_cond_signal(&producer);
  pthread_mutex_unlock(&global_mutex_prod_cons);

  return cell_buffer;
}

/* This is the the code for consumer. We get the
   lines that have been read from producer and extract data */ 

void *getMoviesFromFilesConsumer(void *arg)
{
    int index, nelems, finished;

    /* Arguments que ens han passat */
    struct thread_args *arguments = (struct thread_args *) arg;
    struct buffer *buffer_prodcons = arguments->buffer_prodcons; 

    /* Cell consumer */
    struct cell *cell_consumer;

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
        /* Here we interchange element with the buffer */
        cell_consumer = consumer_get_from_buffer(buffer_prodcons, cell_consumer); 

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
    int i, currentMovieId, index, finished;
    char line[MAXCHAR];

    struct FilesMovies *moviesToReturn;

    struct buffer *buffer_prodcons;
    struct cell *cell_producer;

    /* Reservem memoria per a cell_producer */
    cell_producer = malloc(sizeof(struct cell));

    /* Iniciem el buffer step1 */
    buffer_prodcons = malloc(sizeof(struct buffer));
    buffer_prodcons->read = 0;
    buffer_prodcons->write = 0;
    buffer_prodcons->counter = 0; /* La cel-la no te dades */
    for(i = 0; i < BUFFER_NCELLS; i++) 
        buffer_prodcons->cell_buffer[i] = malloc(sizeof(struct cell));

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

        /* Transferim les dades al buffer */ 
        cell_producer = producer_transfer_to_buffer(buffer_prodcons, cell_producer);

        /* Hem acabat quan ja no hi ha mes dades a llegir del fitxer */
        if (index < CELL_NELEMS)
            finished = 1;
    }

    /* Esperem que acabi el consumidor 1 */
    pthread_join(thread_id, (void *) &moviesToReturn);

    free(cell_producer);
    for(i = 0; i < BUFFER_NCELLS; i++)
        free(buffer_prodcons->cell_buffer[i]);
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
