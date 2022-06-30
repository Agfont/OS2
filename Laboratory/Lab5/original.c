    // 1. Va llegint linea per linea
    /*while(fgets(line, sizeof(line), trainingData)) {
            // 2. Comprova si linea es un ID del Movie
            if(lineIsMovie(line)) {
                if(movieId != -1) { // Entra per cada movie
                    addRatingsToMovie(&currentMovie, currentRatings, currentIndexOfRatings);
                    addMovieToCurrentMovies(currentMovies, &currentMovie, currentIndexOfMovies); //Put ratings too
                    currentIndexOfMovies++;
                    currentIndexOfRatings = 0; // canviem de pelicula
                }
                // Començar amb una nova pelicula
                movieId = getMovieId(line);
                currentMovie.movieId = movieId;
            }
            // Consumidor: Si no es Movie, extreu ID de usuari i rating asociado
            else {
                currentRating = getRating(line);
                currentRatings[currentIndexOfRatings].customerId = currentRating.customerId;
                currentRatings[currentIndexOfRatings].rating = currentRating.rating;
                currentIndexOfRatings++;
            }
        }*/