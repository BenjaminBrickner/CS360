/***********************************************************************
name:
	assignment4 -- acts as a pipe using ":" to seperate programs.
description:	
	See CS 360 Processes and Exec/Pipes lecture for helpful tips.
***********************************************************************/

/* Includes and definitions */
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

/**********************************************************************
./assignment4 <arg1> : <arg2>

    Where: <arg1> and <arg2> are optional parameters that specify the programs
    to be run. If <arg1> is specified but <arg2> is not, then <arg1> should be
    run as though there was not a colon. Same for if <arg2> is specified but
    <arg1> is not.
**********************************************************************/


int main(int argc, char *argv[]) {
	int status;
	int fd[2];
	char *prog1[argc];
	char *prog2[argc];
	
	/*		Argument Checking
	 */
	if (argc == 1) {
		printf("Usage: ./assignment4 <arg1> : <arg2>\n");
		exit (0);
	}
	int i = 0;
	if (strcmp(argv[1], ":") == 0 && argv[2] == NULL) {
		printf("Usage: ./assignment4 <arg1> : <arg2>\n");
		exit (0);
	}
	//get parent arguments
	while ((i < argc-1)) {
		if (argv[i+1] == NULL) {
			break;
		}
		if (strcmp (argv[i+1], ":") == 0) {
			prog1[i] = NULL;
			i++;
			break;
		} else {
			prog1[i] = argv[i+1];
		}
		i++;
	}
	int j = 0;

	//checking right arguments (child)
	while (argv[i+1] != NULL) {
		//check if right argument exists
		if (argv[i+1] == NULL) {
			break;
		}
		prog2[j] = argv[i+1];
		i++;
		j++;
	}
	prog2[j] = NULL;

	//if parent is not provided
	if (prog2[0] == NULL) {
		execvp(prog1[0], prog1);
		printf("%s\n", strerror(errno));
		exit (0);
	}
	if 	(prog1[0] == NULL) {
		execvp(prog2[0], prog2);
		printf("%s\n", strerror(errno));
		exit (0);
	}
	//pipe fd and error check
	if (pipe(fd) < 0) {
		printf("%s\n", strerror(errno));
		exit (errno);
	}
	
	//begin the forking (creating two processes)
	//In fork, you only need to rewire pipes 
	if (fork()) {
	/*
	*	{PARENT'S PROCESS}
	*	Runs with stdout = pipe write end
	*/
		//rewiring parent's FD to only write
		close(0);
		dup (fd[0]);
		close (fd[0]);
		close (fd[1]);
		if (prog1[0] == NULL) exit(0); //do nothing and only execute the child
		execvp(prog2[0], prog2); //execute <arg1>
		printf("%s", strerror(errno));
		exit(0);
	} else { 
	/*
	 *	{CHILD'S PROCESS}
	 *	Runs with stdin = pipe read end
	 */
	 	//rewiring child's FD to only read 
	 	close (1);
		dup (fd[1]); 
		close (fd[1]);
		close (fd[0]);
		execvp(prog1[0], prog1); //execute <arg2>
		printf("%s\n", strerror(errno));
		exit(0);	
	
	}
	return 0;
}
