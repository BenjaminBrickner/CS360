#include "assignment7.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#define SORT_THRESHOLD      40

typedef struct _sortParams {
    char** array;
    int left;
    int right;
} SortParams;

static int maximumThreads;              /* maximum # of threads to be used */
static int thread_num = 0;
pthread_mutex_t mutex;
void errormsg(int errnum);

/* This is an implementation of insert sort, which although it is */
/* n-squared, is faster at sorting short lists than quick sort,   */
/* due to its lack of recursive procedure call overhead.          */

static void insertSort(char** array, int left, int right) {
    int i, j;
    for (i = left + 1; i <= right; i++) {
        char* pivot = array[i];
        j = i - 1;
        while (j >= left && (strcmp(array[j],pivot) > 0)) {
            array[j + 1] = array[j];
            j--;
        }
        array[j + 1] = pivot;
    }
}

/* Recursive quick sort, but with a provision to use */
/* insert sort when the range gets small.            */

static void quickSort(void* p) {
    SortParams* params = (SortParams*) p;
    char** array = params->array;
    int left = params->left;
    int right = params->right;
    int i = left, j = right;
    
    if (j - i > SORT_THRESHOLD) {           /* if the sort range is substantial, use quick sort */

        int m = (i + j) >> 1;               /* pick pivot as median of         */
        char* temp, *pivot;                 /* first, last and middle elements */
        if (strcmp(array[i],array[m]) > 0) {
            temp = array[i]; array[i] = array[m]; array[m] = temp;
        }
        if (strcmp(array[m],array[j]) > 0) {
            temp = array[m]; array[m] = array[j]; array[j] = temp;
            if (strcmp(array[i],array[m]) > 0) {
                temp = array[i]; array[i] = array[m]; array[m] = temp;
            }
        }
        pivot = array[m];

        for (;;) {
            while (strcmp(array[i],pivot) < 0) i++; /* move i down to first element greater than or equal to pivot */
            while (strcmp(array[j],pivot) > 0) j--; /* move j up to first element less than or equal to pivot      */
            if (i < j) {
                char* temp = array[i];      /* if i and j have not passed each other */
                array[i++] = array[j];      /* swap their respective elements and    */
                array[j--] = temp;          /* advance both i and j                  */
            } else if (i == j) {
                i++; j--;
            } else break;                   /* if i > j, this partitioning is done  */
        }
        //j - left to check size
        //only need to thread left side
        SortParams first;  first.array = array; first.left = left; first.right = j;


        pthread_t thread = (pthread_t) NULL;
        int error_check = pthread_mutex_lock(&mutex);
        if (error_check != 0) errormsg(errno);

        /* if there's more threads available and left side has enough words available)*/
        if (thread_num < maximumThreads && (j-left) > SORT_THRESHOLD) { 
            thread_num++;       //increment number of threads in use

            error_check = pthread_mutex_unlock(&mutex); //unlock after changing thread number
            if (error_check != 0) errormsg(errno);

            error_check = pthread_create(&thread, NULL, (void*) quickSort, &first);                  /* sort the left partition  */
            if (error_check != 0) errormsg(errno);
        } else {
            error_check = pthread_mutex_unlock(&mutex); //
            if (error_check != 0) errormsg(errno);

            quickSort(&first); //if no more threads can be made, then go back to old recursion
        }

        SortParams second; second.array = array; second.left = i; second.right = right;
        quickSort(&second);                 /* sort the right partition */

        if (thread != (pthread_t) NULL) {
            error_check = pthread_join(thread, NULL); //waits for parent to finish
            if (error_check != 0) {
                printf("Error in pthread join\n");
                errormsg(errno);
            }
        error_check = pthread_mutex_lock(&mutex);
        if (error_check != 0) errormsg(errno);

        thread_num--; //reduce thread count
        error_check = pthread_mutex_unlock(&mutex);
        
        if (error_check != 0) errormsg(errno);
        }
    } else insertSort(array,i,j);           /* for a small range use insert sort */
}

/*
*       Prints the error message and exits
*/
void errormsg(int errnum) {
    fprintf(stderr, "Error: %s [%d]\n", strerror(errno), errno);
    exit(-1*errno);
}
/* user interface routine to set the number of threads sortT is permitted to use */

void setSortThreads(int count) {
    maximumThreads = count;
}

/* user callable sort procedure, sorts array of count strings, beginning at address array */

void sortThreaded(char** array, unsigned int count) {
    SortParams parameters;
    parameters.array = array; parameters.left = 0; parameters.right = count - 1;
    quickSort(&parameters);
}
