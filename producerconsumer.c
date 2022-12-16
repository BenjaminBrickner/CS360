#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>

pthread_cond_t wantTask;
pthread_cond_t prodTask;
pthread_mutex_t mtx;


char *INPUT = "hello world, this is cs 360!";
char task = '\0';

char produceTask(){
    char c = *INPUT;
    *INPUT++;
    sleep(1);
    return c;
}

void consumeTask(char c){
    printf("%c", c);
    fflush(stdout);
}

void *producer(){
    char temp;
    while(*INPUT != '\0'){
        

        pthread_mutex_lock(&mtx);
        temp = produceTask();
        if(task != '\0'){
            pthread_cond_wait(&prodTask, &mtx);
        }
        //assert(task == '\0');
        task = temp;

        pthread_cond_signal(&wantTask);
        pthread_mutex_unlock(&mtx);
    }

    return NULL;
}

void *consumer(){
    char temp;
    while(1){
        temp = '\0';

        pthread_mutex_lock(&mtx);
        if(task == '\0'){
            pthread_cond_wait(&wantTask, &mtx);

        }
        if(task != '\0'){
            temp = task;
            task = '\0';
            pthread_cond_signal(&prodTask);
        }
        
        if(temp) consumeTask(temp);
        pthread_mutex_unlock(&mtx);

    }
    return NULL;
}

int main(int argc, char const *argv[]){
    pthread_t prod1;
    pthread_t prod2;
    pthread_t cons2;
    pthread_t cons1;

    pthread_mutex_init(&mtx, NULL);
    pthread_cond_init(&wantTask, NULL);
    pthread_cond_init(&prodTask, NULL);

    //pthread_create(&prod1, NULL, producer, NULL);
    pthread_create(&prod2, NULL, producer, NULL);
    pthread_create(&cons1, NULL, consumer, NULL);
    pthread_create(&cons2, NULL, consumer, NULL);

    //pthread_join(prod1, NULL);
    pthread_join(prod2, NULL);
    
    pthread_join(cons1, NULL);
    pthread_join(cons2, NULL);


    return 0;
}