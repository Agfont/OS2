/* Funcions thread-safe a fer servir */

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

