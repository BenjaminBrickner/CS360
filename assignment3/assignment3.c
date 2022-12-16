#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
/*	Assignment
 * Take as input some path. Recursively count the files that are readable.
 * If a file is not readable, do not count it. Return the negative value
 * of any error codes you may get. However, consider what errors you want
 * to respond to.
 */
/*note:
 * This function is recursive
*/
int readable(char* inputPath) {
	int readable_files = 0;
	struct dirent x, *directP = &x;
	

	//if ithe input is null, set it to the current directory
	if (inputPath == NULL) {
		inputPath = ".";
	}

	//lstat obtains info about the path
	if (lstat (inputPath, s) == 0)	{

		//check if it's directory
		if (s->st_mode & S_IFDIR) {
			DIR* directory = opendir(inputPath);
			if (directory == NULL) return (0);
						
			//check if directory is not searchable 
			if (access(inputPath, X_OK)) {
				return 0; //no add to the counter and don't traverse in the directory
			}

			if (chdir(inputPath) < 0) return (-1*errno);
			//traverse each file in directory
			while ((directP = readdir(directory)) != NULL) {
				if (directP->d_name[0] !=  '.') {
					//do not go through another recursion if readable encountered a negative number (an error)
					if (readable_files < 0) {
						return (readable_files);
					}	
					readable_files += readable(directP->d_name); //recall function with file name and increment 
				}		
			}

			if (closedir(directory) < 0) return (-1*errno);
			if(chdir("..") < 0) return (-1*errno);

		//check if it's a regular file
		} else if (s->st_mode & S_IFREG) {
			//check if its accessable for read
			if (!(access(inputPath, R_OK))) {
			        return 1;	
			} else {
				return 0;
			}

		//if not a directory nor a file	
		
		} else {
			return (-1*errno);
		}

	//cannot obtain info about the path
	} else {
		return(-1*errno);
	}

	return (readable_files);
}
