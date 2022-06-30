#include "RecommendationMatrix.h"
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <pthread.h>

#define MAXIMUM_VALUE_OF_KEYS_HASH_TABLE 600000
#define EPSILON_COMPARISON 0.00001f
#define M 2
#define N 10000
pthread_t ntid[M];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int last_block = 0;

void createHashMoviesAndUsers(int numberOfUsers, int numberOfMovies) {
    hcreate((int)(1.25*(numberOfUsers+numberOfMovies)));
}

void removeMoviesAndUsersFromTable(int numberOfMovies, struct Movie* movies, int numberOfUsers, struct User* users) {
    ENTRY pairRow, *ep;
    char key[10];
    char **keysToFree = malloc(MAXIMUM_VALUE_OF_KEYS_HASH_TABLE*sizeof(char*));
    int currentKeys = 0;
    for(int i=0; i<numberOfMovies; i++) {
        sprintf(key, "m%d", movies[i].movieId);
        pairRow.key = key;
        ep = hsearch(pairRow, FIND);
        if (ep != NULL) {
            keysToFree[currentKeys] = ep->key;
            currentKeys++;
        }
    }
    for(int i=0; i<numberOfUsers; i++) {
        sprintf(key, "u%d", users[i].userId);
        pairRow.key = key;
        ep = hsearch(pairRow, FIND);
        if (ep != NULL) {
            keysToFree[currentKeys] = ep->key;
            currentKeys++;
        }
    }
    keysToFree = (char **) realloc(keysToFree, currentKeys*sizeof(char*));
    if(keysToFree == NULL) {
        printf("Memory management failed. Contact professors\n");
        exit(1);
    }
    for(int i=0; i < currentKeys;i++) {
        free(keysToFree[i]);
    }
    free(keysToFree);
}

void destroyHashMoviesAndUsers() {
    hdestroy();
}

void addUsersToTable(int numberOfUsers, struct User* users) {
    ENTRY pairUserRow, *ep;
    char key[10];
    for(int i=0; i<numberOfUsers; i++) {
        sprintf(key, "u%d", users[i].userId);
        pairUserRow.key = strdup(key);
        pairUserRow.data = (void *) (intptr_t) i;
        ep = hsearch(pairUserRow, ENTER);
        if (ep == NULL) {
            fprintf(stderr, "entry failed\n");
            exit(EXIT_FAILURE);
        }
    }
}

void addMoviesToTable(int numberOfMovies, struct Movie* movies) {
    ENTRY pairMovieCol, *ep;
    char key[10];
    for(int i=0; i<numberOfMovies; i++) {
        sprintf(key, "m%d", movies[i].movieId);
        pairMovieCol.key = strdup(key);
        pairMovieCol.data = (void *) (intptr_t) i;
        ep = hsearch(pairMovieCol, ENTER);
        if (ep == NULL) {
            fprintf(stderr, "entry failed\n");
            exit(EXIT_FAILURE);
        }
    }
}

int lookRowForUser(int userId) {
    ENTRY pairUserRow, *ep;
    char key[10];
    sprintf(key, "u%d", userId);
    pairUserRow.key = key;
    ep = hsearch(pairUserRow, FIND);
    // It returns the row only if it exists
    return ep ? (int)(intptr_t)(ep->data) : -1;
}

int lookColForMovie(int movieId) {
    ENTRY pairMovieCol, *ep;
    char key[10];
    sprintf(key, "m%d", movieId);
    pairMovieCol.key = key;
    ep = hsearch(pairMovieCol, FIND);
    // It returns the row only if it exists
    return ep ? (int)(intptr_t)(ep->data) : -1;
}

struct RecommendationMatrix createEmptyRecommendationMatrix(int numberOfUsers, int numberOfMovies) {
    long matrixSize = numberOfUsers*numberOfMovies*sizeof(char);
    long nelems = numberOfUsers*numberOfMovies;
    struct RecommendationMatrix matrix;
    char *grid = (char *) mmap(NULL, matrixSize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if(grid == MAP_FAILED) {
        printf("Error allocating matrix. Aborting\n");
        exit(1);
    }
    matrix.grid = grid;
    matrix.numberOfRows = numberOfUsers;
    matrix.numberOfColumns = numberOfMovies;
    memset(matrix.grid, 0, nelems);
    return matrix;
}

/*
 * 0 means that the user has not given a rate for that movie. Ratings range is [1, 5]\int Z. It is not working for some reason
 */
void fillRecommendationMatrix(int numberOfMovies, struct Movie* movies, struct RecommendationMatrix *recommendationMatrix) {
    int row, column;
    for(int i=0; i < numberOfMovies; i++) {
        for(int j=0; j < movies[i].numberOfRatings; j++) {
            row = lookRowForUser(movies[i].ratings[j].customerId);
            column = lookColForMovie(movies[i].movieId);
            setRecommendationMatrixValue(row, column, movies[i].ratings[j].rating, recommendationMatrix);
        }
    }
}

float accessRecommendationMatrixValue(int row, int col, struct RecommendationMatrix *recommendationMatrix) {
    return recommendationMatrix->grid[row * recommendationMatrix->numberOfColumns + col];
}

void setRecommendationMatrixValue(int row, int col, char value, struct RecommendationMatrix *recommendationMatrix) {
    recommendationMatrix->grid[row * recommendationMatrix->numberOfColumns + col] = value;
}

float _getAccSum(int rowUser1, int rowUser2, struct RecommendationMatrix *recommendationMatrix) {
    char *user1, *user2;
    float ratingUser1, ratingUser2, accSum = 0;
    user1 = recommendationMatrix->grid + rowUser1 * recommendationMatrix->numberOfColumns;
    user2 = recommendationMatrix->grid + rowUser2 * recommendationMatrix->numberOfColumns;
    for(int col = 0; col < recommendationMatrix->numberOfColumns; col++) {
        ratingUser1 = *user1;
        ratingUser2 = *user2;
        if(ratingUser1 > 0 && ratingUser2 > 0)
            accSum += pow(ratingUser2 - ratingUser1, 2);
        user1 += 1;
        user2 += 1;
    }
    return accSum;
}

float similarityUsers(int rowUser1, int rowUser2, struct RecommendationMatrix *recommendationMatrix) {
    //P.e la euclidiana
    float accSum = _getAccSum(rowUser1, rowUser2, recommendationMatrix);
    return 1.0/(1.0 + sqrt(accSum));
}

float forecastRating(int colMovieToScore, int rowUser, struct RecommendationMatrix *recommendationMatrix) {
    char *pointerCurrentRating;
    float currentRating, currentSimilarity, accSimilarity, accRating;
    accSimilarity = 0;
    accRating = 0;
    pointerCurrentRating = recommendationMatrix->grid + colMovieToScore;
    for(int rowAnotherUser=0; rowAnotherUser < recommendationMatrix->numberOfRows; rowAnotherUser++) {
        if(rowAnotherUser != rowUser) {
            currentRating = *pointerCurrentRating; //Accessing via pointers
            if(currentRating > 0) {
                currentSimilarity = similarityUsers(rowUser, rowAnotherUser, recommendationMatrix);
                accSimilarity += currentSimilarity;
                accRating += currentSimilarity*currentRating;
            }
        }
        pointerCurrentRating += recommendationMatrix->numberOfColumns;
    }
    return accRating/((float)(accSimilarity));
}

int getNumberOfMoviesSeenByUser(int rowUser, struct RecommendationMatrix *recommendationMatrix) {
    char *pointerCurrentRating, *maxPointerToIterate;
    float currentRating;
    int numberOfMoviesSeenByUser = 0;
    pointerCurrentRating = recommendationMatrix->grid + rowUser*recommendationMatrix->numberOfColumns;
    maxPointerToIterate = pointerCurrentRating + recommendationMatrix->numberOfColumns;
    for(;pointerCurrentRating < maxPointerToIterate; pointerCurrentRating++) {
        currentRating = *pointerCurrentRating; //Accessing via pointers
        if(currentRating > 0)
            numberOfMoviesSeenByUser++;
    }
    return numberOfMoviesSeenByUser;
}

int getNumberOfUsersThatHaveSeenMovie(int colMovie, struct RecommendationMatrix *recommendationMatrix) {
    char *pointerCurrentRating;
    float currentRating;
    int numberOfUsersThatHaveSeenTheMovie = 0;
    pointerCurrentRating = recommendationMatrix->grid + colMovie;
    for(int row = 0; row < recommendationMatrix->numberOfRows; row++) {
        currentRating = *pointerCurrentRating; //Accessing via pointers
        if(currentRating > 0)
            numberOfUsersThatHaveSeenTheMovie++;
        pointerCurrentRating += recommendationMatrix->numberOfColumns;
    }
    return numberOfUsersThatHaveSeenTheMovie;
}

struct retorn {
    int recommendedMovieColumn;
    float score;
};

struct parametres { 
    int rowUser;
    struct RecommendationMatrix *m;
};

void *thr_fn(void *arg) {
    int recommendedMovieColumn = -1;
    float maxScore = -1;
    int col_begin = -1 , col_end = -2;
    float currentScore = 0;
    float matrixValue;
    
    struct parametres *par = (struct parametres *) arg;
    int rowUser = par->rowUser;
    struct RecommendationMatrix *recommendationMatrix = par->m;
    
    struct retorn *ret;
    ret = malloc(sizeof(struct retorn));
    
    while (col_begin != col_end) {
        pthread_mutex_lock(&mutex);
        col_begin = last_block;
        col_end = col_begin + N;
        if (col_end > recommendationMatrix->numberOfColumns) {
            col_end = recommendationMatrix->numberOfColumns;
        }
        last_block = col_end;
        pthread_mutex_unlock(&mutex);

        for(int j=col_begin; j < col_end; j++) {
            // We only accept movies that has not been seen yet.
            matrixValue = *(recommendationMatrix->grid + j + rowUser*recommendationMatrix->numberOfColumns);
            if(fabs(matrixValue) <= EPSILON_COMPARISON) {
                currentScore = forecastRating(j, rowUser, recommendationMatrix);
                if(currentScore > maxScore) {
                    maxScore = currentScore;
                    recommendedMovieColumn = j;
                }
            }
        }
    }
    ret->recommendedMovieColumn = recommendedMovieColumn;
    ret->score = maxScore;
    return ((void *) ret);
}

/**
 * It returns the movieId of the recommended movie. If the user has seen all the possible movies it returns -1.
 */
int getRecommendedMovieForUser(int rowUser, struct RecommendationMatrix *recommendationMatrix, struct Movie* movies, int numberOfMovies) {
    struct retorn *tret; //void *tret;
    int err;
    int recommendedMovieColumn = -1;
    float maxScore = -1;

    struct parametres par;
    par.m = recommendationMatrix;
    par.rowUser = rowUser;
    
    for(int i = 0; i < M; i++)
        err = pthread_create(&(ntid[i]), NULL, thr_fn, (void *) &par);
    for(int i = 0; i < M; i++) {
        err = pthread_join(ntid[i], (void *) &tret); // Argument capturat
        if ((float) tret->score > maxScore) {
            maxScore = (float) tret->score;
            recommendedMovieColumn = (int) tret->recommendedMovieColumn;
        }
        free(tret);
    }
    
    // The user has seen all the movies, we return -1
    if(recommendedMovieColumn == -1)
        return -1;
    for(int i = 0; i < numberOfMovies; i++) {
        if(lookColForMovie(movies[i].movieId) == recommendedMovieColumn)
            return movies[i].movieId;
    }

    return 0;
}

/**
 * Note: You need to free the matrix if you have allocated memory for it. This only free internals.
 * @param recommendationMatrix
 */
void freeRecommendationMatrix(struct RecommendationMatrix *recommendationMatrix) {
    if(munmap(recommendationMatrix->grid,
              recommendationMatrix->numberOfRows*recommendationMatrix->numberOfColumns*sizeof(char)) == -1) {
        printf("Error deallocating matrix. Aborting\n");
        exit(1);
    }
}
