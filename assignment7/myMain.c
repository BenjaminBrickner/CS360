#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "assignment7.h"

#define THREADS 1
#define LINE_NUM 10000

char* getNextWord(FILE* fd);

void main() {
    int wordCounter = 0;
    char **temp;
    temp = malloc(sizeof(char*)*LINE_NUM);
    FILE* fd = fopen("webster_random", "r");
    for (int i  = 0; i < LINE_NUM; i++) {
        temp[i] = getNextWord(fd);
        wordCounter++;
    }
    fclose(fd);
    setSortThreads(THREADS);
    struct timeval start, end;
    gettimeofday(&start, NULL);
    sortThreaded(temp, wordCounter);
    gettimeofday(&end, NULL);
    //printf("%ld\n", (end.tv_sec * 10000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
    for(int i = 0; i < LINE_NUM; i++) {
        printf("%s\n", temp[i]);
        free(temp[i]);
    }
    free(temp);
}
