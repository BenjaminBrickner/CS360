#include "main.h"

//two header files provided by instructor
#include "getWord.h"
#include "crc64.h"

int main(int argc, char *argv[]) {
  int argument_count = -1;
  FILE *fp;
  char* word1;
  char* word2;
  hashArray hashTable = init(HASHSIZE);

  /*		Argument Checking
   *  The valid argument according to assignment is  
   *  wordPair <-count> fileName <fileName2> <fileName3> ...  where <> are optional 
   */


  //in case someone typed so many arguments for the program to run :P
  if (argc > 10) {
     fprintf(stderr, "That's too many arguments!\n");
     fprintf(stderr, "Usage: ./wordpairs <-count> fileName1 <fileName2> <fileName3> ...\n");
     return -1;
  }


  if (argc == 2) {  //one argument specified
     
     //edge case if entered a number instead of 
     fp = fopen(argv[1], "r"); //open the only file for reading  
     if (fp == NULL) {
	fprintf(stderr, "File cannot be opened. Usage: ./wordpairs <-count> fileName1 <fileName2> <fileName3> ...\n");
	hashFree(hashTable);
        return -1;
      }
      //build iniiial table first
      hashTable = buildTable(fp, hashTable);
      //if table needs to be expanded, expand the table
      while (hashTable.isTooSmall) {
        hashTable = newHashTable(hashTable, fp);
     } 
     fclose(fp); //close after done using file



  } else if (argc >= 3) {  //two or more arguments specified <-count> and fileName <fileName2> <fileName3> ...
     //check count argument
     if (sscanf(argv[1], "-%d", &argument_count) == 1) {
	if (argument_count < 0) {
	   //argument cannot be negative
	   fprintf(stderr, "The count number cannot be less 1.\n");
	   fprintf(stderr, "Usage: ./wordpairs <-count> fileName1 <fileName2> <fileName3> ...\n");
	   hashFree(hashTable);
	   return -1;
	}
        //check if it's a file instead
        for (int i = 3; i <= argc; i++) {
	   fp = fopen(argv[i-1], "r");
 	   if (fp == NULL) {
	      //fp is null, file doesn't not exist, standard error
	      fprintf(stderr, "Usage: ./wordpairs <-count> fileName1 <fileName2> <fileName3> ...\n");
	      hashFree(hashTable);
	      return -1;
	   }

	   hashTable = buildTable(fp, hashTable);
	   while (hashTable.isTooSmall) {	
              hashTable = newHashTable(hashTable, fp);
	   }
	   fclose(fp); //done with previous fp
        } 

    //else statement if first argument is NOT a <-count> argument
     } else {
	//for liop to iterate from file to file arguments
	for (int i = 2; i <= argc; i++) {
	   fp = fopen(argv[i-1], "r");
	   if (fp == NULL) {
	   //fp is null, standard error
		fprintf(stderr, "Usage: ./wordpairs <-count> fileName1 <fileName2> <fileName3> ...\n");
		hashFree(hashTable);
		return -1;
	   }
	   hashTable = buildTable(fp, hashTable);
	   while (hashTable.isTooSmall) {
                hashTable = newHashTable(hashTable, fp);
	   }
	   fclose(fp);
	}
     }


  //else statement if argc is less than 2 (no arguments)     
  } else {
	//too little arguments, standard error
	fprintf(stderr, "Enter at least a file name: wordPair <-count> fileName1 <fileName2> <fileName3>\n");
	hashFree(hashTable);
	return -1;
  }

  struct node* qsortArray = sortArray(hashTable);

  //sorting the array with quicksort in respect to the count number
  
  qsort(qsortArray, hashTable.total_pairs, sizeof(struct node), compare); 
  
  int listLength = 0;
  //checking if argument is given by integer flag. And assign listLength accordingly
  if (argument_count != -1) {
       listLength = argument_count;	
  } else {
       //if argument was not given (still at -1 flag)
       listLength = hashTable.total_pairs;
  }

  //listLength cannot be larger than the length of all entries
  if (listLength > hashTable.total_pairs) {
        fprintf(stderr, "Hey. You entered a number that's greater than the number of paired words.\n");
	fprintf(stderr, "Here's everything in ths list...\n");
	listLength = hashTable.total_pairs;
  }


  listPrint(qsortArray, listLength); //prints list to stdout

  //free and close after done using
  hashFree(hashTable);
  free(qsortArray);
  return 0;
}

/*		compare function
 * this is for the "qsort" algorithm to compare two values and return an int depending on the values in the count
 */
int compare (const void* a, const void* b) {
	struct node* A = (struct node *)a;
	struct node* B = (struct node *)b; 
	return(B->count - A->count);
}
