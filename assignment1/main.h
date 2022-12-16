#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//change the variable number below to change the default hash table size
#define HASHSIZE 10 

//this is the node for linked list
struct node {
  char *pair_of_words;
  int count;
  int listSize;
  struct node *next;
};

typedef struct node* wordPairEntry;

//struct for hash itself including the hashTable array
typedef struct hash {
  bool isTooSmall;
  int total_pairs;
  int hashSize;
  wordPairEntry* hashTable;
} hashArray;

hashArray init(int size); //creates an initial array of linked lists
			  
struct node* LL_init(); //creates linked list for specific array

bool listAdd(struct node* sent, char* newPairOfWords, bool isCopy, int copy_count); //adds a pair of words at beginning of linked list

bool isRepeat(struct node* sent, char* newWordPair); //checks if same pair of words appear

hashArray buildTable(FILE *fp, hashArray hashTable); //builds hash table from the file on top of initializing it

void hashFree(hashArray hashTable); //free the entire hash table from index 0 to index of its size

void LL_free(struct node *sent); //frees the linked list in specific index
				 
void listPrint(struct node sortedArray[], int sortSize); //loops through the sorted array and prints the repeat number and  the pair of word associated 

int compare(const void* a,const void* b); //a qsort function that sorts it from largest to lowest repeat number

struct node* sortArray(hashArray hashTable); //makes a new array of struct nodes from the hashTable

struct node* getList(struct node* sent, int listLength); //gets the list for the sort array function

hashArray newHashTable (hashArray hashTable, FILE *fp); //grows hashtable and calls buildtable with new size

void copyHashIndex(hashArray biggerHashtable, struct node* oldSent, bool isCopy); //copies from old hash table to new
