#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define PHILO_NUM 5
#define EAT_TIME 9 //+- 3 (Std Dev)
#define THINK_TIME 11 //+- 7 (Std Dev)
#define MAX_EAT_TIME 100

void philosopher(void* philo_id);
int randomGaussian(int mean, int stddev);

pthread_mutex_t stick[PHILO_NUM + (PHILO_NUM == 1)]; //initialize each mutex. If there's only one philosopher, initialize two sticks 


/*
           						 My Main Program

	Starts the program with declaring and making threads representing philosophers around the table.
	
*/
int main(int argc, char *argv[]){
	
	pthread_t philo[PHILO_NUM]; //initialize each thread
	int errval;
	if(errval = pthread_mutex_init(stick, NULL) != 0) {
		fprintf(stderr, "Error when using pthread_mutex_init: %s [%d]\n", strerror(errval), errval);
		return -1;
	}

	//initialize an array of philosophers ID 
	int id[PHILO_NUM] = {0};

	//creating threads for each philosopher
	for (int i = 0; i < PHILO_NUM; i++) {
		id[i] = i;
		if(errval = pthread_create(&philo[i], NULL, (void* )philosopher, &id[i]) != 0) {
			fprintf(stderr, "Error: Can't create thread: %s [%d]\n", strerror(errval), errval);
			return -1;
		}
	}
	
	//waiting for each thread to end
	for (int i = 0; i < PHILO_NUM; i++) {
		if (errval = pthread_join(philo[i], NULL) != 0) {
			fprintf(stderr, "Error: Failed to join/wait thread: %s [%d]\n", strerror(errval), errval);
			return -1;
		}
	}
	return 0;
}

/*
						Philosopher Function
	This is the core functionality of each philosopher in their own thread created in main. 
	"philo_id" is the argument passed from pthread_create() which contains an address of the philosopher ID integer
*/

void philosopher(void* philo_id) {
	int total_time = 0;
	int sleep_num = 0;
	int try1;
	int try2;

	//make random seed for each philosopher
	unsigned int random_seed = time(NULL);
	srand(random_seed + *(int*)philo_id);

    /*
	 * 			The main while loop for each philosopher until their eating time is up
	*/
    while (total_time < MAX_EAT_TIME) {
		//get random number

		sleep_num = randomGaussian(THINK_TIME, 7); //get random thinking time
		if (sleep_num < 0) sleep_num = 0;

		printf("Philosopher #%d is thinking for %d seconds. Total eating time = %d\n", *(int*) philo_id, sleep_num, total_time);
		sleep(sleep_num);

		/*
		 *			While loop for waiting for both sticks to be available, if both sticks are available, 
		 *			then this loop will break and the eating section begins. 
		*/
		do {
			/*	try to lock sticks, if the stick is occupied by another thread (or philosopher in this example),
			*	try1/try2 will return an error number with macro "EBUSY"
			*/

			try1 = pthread_mutex_trylock(&stick[*(int*)philo_id]);
			try2 = pthread_mutex_trylock(&stick[(*(int*)philo_id+1)%PHILO_NUM + (PHILO_NUM == 1)]);

				//put down one or the other stick if the other stick is occupied
				if (try1 == EBUSY && try2 != EBUSY) {
					pthread_mutex_unlock(&stick[(*(int*)philo_id+1)%PHILO_NUM + (PHILO_NUM == 1)]);
				}
				if (try2 == EBUSY && try1 != EBUSY) {
					pthread_mutex_unlock(&stick[*(int*)philo_id]);
				}
		} while (try1 == EBUSY || try2 == EBUSY);

		//get random eat time and increment total eating time
		sleep_num = randomGaussian(EAT_TIME, 3);
		total_time += sleep_num;

		printf("Philosopher #%d is eating for %d seconds.  Total eating time = %d\n", *(int*) philo_id, sleep_num, total_time);
		fflush(stdout);
		sleep(sleep_num);

		//unlock both sticks for availability
		pthread_mutex_unlock(&stick[*(int*)philo_id]);
		pthread_mutex_unlock(&stick[(*(int*)philo_id+1)%PHILO_NUM + (PHILO_NUM == 1)]);
	}
	printf("Philosopher #%d is finished and leaving. Eating time = %d\n", *(int*) philo_id, total_time);	
	fflush(stdout);
	return; //end thread if the eating time is reached
}


/*				randomGaussian
*	Given by instructor to calculate a number for sleep
*/

int randomGaussian(int mean, int stddev) {
	double mu = 0.5 + (double) mean;
	double sigma = fabs((double) stddev);
	double f1 = sqrt(-2.0 * log((double) rand() / (double) RAND_MAX));
	double f2 = 2.0 * 3.14159265359 * (double) rand() / (double) RAND_MAX;
	if (rand() & (1 << 5)) 
		return (int) floor(mu + sigma * cos(f2) * f1);
	else            
		return (int) floor(mu + sigma * sin(f2) * f1);
}