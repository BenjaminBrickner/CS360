#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/errno.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>


//You can change the number of philosophers here
#define PHILO_NUM 5
#define EAT_TIME 9 //+- 3 (Std Dev)
#define THINK_TIME 11 //+- 7 (Std Dev)
#define MAX_EAT_TIME 15

int randomGaussian(int mean, int stddev);
int philosopher(int id, int philo_num);

/*				MAIN
*	This is the main function that initializes the semaphores, and forks multiple children
*	Each children will call the philosopher function to being eating and thinking 
*/

int main(int argc, char *argv[])
{
	/*
	 *		Set times for eating and thinking
	 */

	unsigned short philo[PHILO_NUM] = {0};
	int semID = semget(IPC_PRIVATE, PHILO_NUM, IPC_CREAT | IPC_EXCL | 0655); 

	//checking semget error
	if (semID == -1) {
		fprintf(stderr, "SemID returned an error: %s\n", strerror(errno));
		exit(-1);
	}

	 //initializes all value to 0 and error checks
	if (semctl(semID, 0, SETALL, philo) < 0) {
		fprintf(stderr, "Initial semget failed: %s\n", strerror(errno));
		exit(0);
	}

	int ret;

	//The forking process
	for (int i = 0; i < PHILO_NUM; i++) {
		ret = fork();
		if (ret == -1) {
			fprintf(stderr, "Fork returned an error: %s\n", strerror(errno));
			exit(0);
		}
		if (ret == 0) {
			int total_eating_time = philosopher(semID, i);
			printf("Philosopher %d is done. Total eating time was %d seconds\n", i, total_eating_time);
			exit(0);
		} 
	}
		//wait for children
	for (int i = 0; i < PHILO_NUM; i++)
		wait (NULL);

	//removes semaphore + error check
	//Dev note: the semnum argument (0) is ignored from the IPC operation
	if (semctl(semID, 0, IPC_RMID) < 0) {
		fprintf(stderr, "Semaphore closing error: %s\n", strerror(errno));
	}  
}


/*				philosopher function
*	Makes a pair of sembuf operations (one for each chopstick)
*	This is what each child (or philosopher) 
*/				

int philosopher(int id, int philo_num) {
	struct sembuf eat[2] = {{philo_num,-1,0}, {(philo_num+1)%PHILO_NUM, -1, 0}};
	struct sembuf think[2] = {{philo_num,1,0}, {(philo_num+1)%PHILO_NUM, 1, 0}};
	//Dev note: Use modular operator to have the last stick be the same stick as the first

	int sleep_num;
	int total_eat = 0;
	int total_think = 0;

	semop(id, think, 1); //inital semaphore to think
	while (1) {
		//getting random value for waiting time
		srand(philo_num);
		sleep_num = randomGaussian(THINK_TIME, 7);
		if (sleep_num < 0) sleep_num = 0;

		
		printf("Philosopher %d is thinking for %d seconds (total = %d)\n", philo_num, sleep_num, total_eat);
		fflush(stdout);
		sleep(sleep_num);
		semop(id, eat, 2);

		//		After getting signal to eat, begin eating
		
		//getting random value for eating time
		srand(philo_num);
		sleep_num = randomGaussian(EAT_TIME, 3);
		if (sleep_num < 0) sleep_num = 0; //randomGaussian can return a negative

		printf("Philosopher %d is eating for %d seconds (total = %d)\n", philo_num, sleep_num, total_eat);
		fflush(stdout);
		sleep(sleep_num);
		total_eat += sleep_num; 

		semop(id, think, 2); //await signal for waiting

		//checking if the child has reached their maximum eat time
		if (total_eat >= MAX_EAT_TIME) return total_eat; 
	}
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
