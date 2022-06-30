/*
 * This main is not intended to be used, but if you want to experiment you can do it with the data attached in the 'data'
 * folder.
 */

#include <stdio.h>
#include <stdlib.h>
#include "model/CSVReader.h"
#include "model/User.h"
#include "model/RecommendationMatrix.h"
#include "model/Movie.h"
#include "model/MatrixDiskService.h"
#include "model/HashTableDiskService.h"

int mainMultipleFiles(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Number of files used not specified as argument, try again.");
        exit(EXIT_FAILURE);
    }
    int numberOfFiles = atoi(argv[1]);
    char filename[200];
    struct FileMovies fileMovies[numberOfFiles];
    for (int i = 1; i < numberOfFiles+1; i++) {
        sprintf(filename, "../data/combined_data_%d.txt", i);
        fileMovies[i-1] = getMoviesFromCSV(filename);
    }
    struct User *uniqueUsers;
    int numberOfUniqueUsers;
    struct Movie *uniqueMovies;
    int numberOfUniqueMovies;
    struct UniqueUsersResult uniqueUsersResult = getUniqueUsers(numberOfFiles, fileMovies);
    uniqueUsers = uniqueUsersResult.userList;
    numberOfUniqueUsers = uniqueUsersResult.numberOfUsers;
    struct UniqueMoviesResult uniqueMoviesResult = getUniqueMovies(numberOfFiles, fileMovies);
    uniqueMovies = uniqueMoviesResult.moviesList;
    numberOfUniqueMovies = uniqueMoviesResult.numberOfMovies;
    createHashMoviesAndUsers(numberOfUniqueUsers, numberOfUniqueMovies);
    addUsersToTable(numberOfUniqueUsers, uniqueUsers);
    addMoviesToTable(numberOfUniqueMovies, uniqueMovies);
    struct RecommendationMatrix recommendationMatrix = createEmptyRecommendationMatrix(numberOfUniqueUsers, numberOfUniqueMovies);
    fillRecommendationMatrix(numberOfUniqueMovies, uniqueMovies, &recommendationMatrix);
    // =====================
    // Saving matrix and hash values into disk
    saveMatrixIntoDisk("./matrix.bin", "./meta.bin", &recommendationMatrix);
    saveHashTableMoviesAndUsers("./hash.bin", numberOfUniqueUsers, uniqueUsers, numberOfUniqueMovies, uniqueMovies);
    // =====================
    // Freeing hash
    removeMoviesAndUsersFromTable(numberOfUniqueMovies, uniqueMovies, numberOfUniqueUsers, uniqueUsers);
    freeRecommendationMatrix(&recommendationMatrix);
    // FREE MEMORY OF MOVIE FILES: We do not need it anymore
    for (int i = 0; i < numberOfFiles; i++) {
        freeFileMovies(&fileMovies[i]);
    }
    destroyHashMoviesAndUsers();
    //Ending program
    for(int i=0; i < numberOfUniqueMovies; i++) {
        freeMovie(&uniqueMovies[i]);
    }
    free(uniqueMovies);
    free(uniqueUsers);
}