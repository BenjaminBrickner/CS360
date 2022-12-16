#include "main.h"
#include "getWord.h"
#include "crc64.h"
/*      HashTable Init
* Makes a HashTable array with each value being a linked list
*/
hashArray init(int size) {
  hashArray x;
  x.isTooSmall = false; //default to false unless list size is over limit
  x.hashTable = malloc(size*sizeof(wordPairEntry)); //allocate memory for hashTablie
  x.hashSize = size;
  x.total_pairs = 0;
  for (int i = 0; i < size; i++) {
    x.hashTable[i] = LL_init();
  }
  return (x);

}


/*        Linked List Init
*   Makes a sentinel node for an entree of the hashTable
*/
struct node* LL_init() {
  struct node *sent;
  sent = malloc(sizeof(struct node));

  //setting values to null for sentinel node
  sent -> count = 0; //NOTE: setting to null gives a warning
  sent -> pair_of_words = NULL;
  sent -> listSize = 0;
  sent -> next = NULL;
  return (sent);
}

/*		listAdd
 *  adds the pair of words to hashTable in a linked list
 */
bool listAdd(struct node *sent, char* newPairOfWords, bool isCopy, int copy_count) {
 
   //function that check if there is already a pair of word in the linked list
   if (isRepeat(sent, newPairOfWords)) return true; 


   struct node *newPair = malloc(sizeof(struct node));
   newPair -> pair_of_words = malloc(strlen(newPairOfWords)+1);
   newPair -> count = 1; //set number of time repeated to one

   //check if we're copying to new array. If yes, copy the old counter to new in order to avoid loss in counting
   if (isCopy) newPair -> count = copy_count;

   //copy new info 
   strcpy(newPair -> pair_of_words, newPairOfWords); 
   //placing newPair node at beginning to Linked List
   newPair -> next = sent -> next;
   sent -> next = newPair;
   if (newPair -> next != NULL) {
       newPair -> listSize = newPair -> next -> listSize + 1;
   } else {
       newPair -> listSize = 1;
   }
   return false;
}

/*		isRepeat
 * passes the pointer to first element in linked list and the string to compare
 * returns TRUE of the string matches any of the string in the linked list
 * return FALSE if there's the string is a new string
 */

bool isRepeat(struct node* sent, char* newWordPair) {
   //temperary traversing variables
   struct node* prev = sent;
   struct node* curr = sent -> next;

   //checks the linked list for matches
   while (curr != NULL) {
      
      //if same two words appear, add one to the count and return true to avoid mallocing and making a new node
      if (strcmp(curr->pair_of_words, newWordPair) == 0) {
         curr -> count += 1;	      
         return true;
      }
      //traverse to next node in linked list
      prev = prev -> next;
      curr = curr -> next;
   }
   //if nothing found, return false by default
   return false;
}

/*		buildTable (initialize build)
 *  This function takes a file and hashTable arguments
 *  It reads from the file pointer given, and begins adding the words to the table
 */

hashArray buildTable(FILE *fp, hashArray hashTable) {				 
   //targetting previous and current word to itterate
   char *prevWord;
   char *currWord;
   prevWord = getNextWord(fp);
   hashTable.isTooSmall = false; //default
   while (true) {
      currWord = getNextWord(fp); //grabs the word 

      //breaks out of loop
      if (currWord == NULL) break;    

       char* wordPair = malloc(strlen(prevWord)+1 + strlen(currWord)+1 + 1); //making word pair with allocated space based on length of two strings and one for the space between the two
      //combining the two words with an empty space
         strcpy(wordPair, prevWord);
         strcat(wordPair, " ");
         strcat(wordPair, currWord);

	 int hash_num = crc64(wordPair) % hashTable.hashSize; //use given hash function to get index

         //crc64 returns a hash number for the table, modular that by table size, and pass the pair of words to add to the linked list
         bool isDuplicate = listAdd(hashTable.hashTable[hash_num], wordPair, false, 0);
	 
	 //after adding to list, increment total_pair variable (not duplicate)
	 if (!isDuplicate) { 
	      hashTable.total_pairs++;

	      //check if it's null before checking collision
	      if (hashTable.hashTable[hash_num]->next != NULL) {


		    /*							Number of Collisions
		     * I decided to pick 10 as my limit, because I want the hash table to focus more on inserting than growing, since growing is a long process
		     */

		    if (hashTable.hashTable[hash_num]->next->listSize > 10) {
			free(prevWord);
			free(currWord);
			free(wordPair);
			hashTable.isTooSmall = true; //set the flag to be true and return
			return (hashTable);
		    } 
		}
	 }

         //free and traverse
         free(prevWord);
         prevWord = currWord;
         free (wordPair); //done mallocing wordPair after copying to struct in linked list
	
     }
     //free final words
     free(prevWord);
     free(currWord);
     
     return (hashTable);
}

/*		sortArray 
 * Takes an argument of the hashArray struct
 * Creates a new array for the qsort function
 * This should only have to run once for the table initial building
 * Returns an array of struct nodes
 */

struct node* sortArray(hashArray hashTable) {
     //allocating temp to be the array of struct nodes with the size of length "total_pairs"
     struct node* temp = malloc(sizeof(struct node)*hashTable.total_pairs); 
     struct node* entryList;
     int listSize;

     int temp_index = 0;


     //  This entire loop is for copying the entire hashTable of linked list into an array for qsort
     //

     for (int i = 0; i < hashTable.hashSize; i++) {

	 //This small conditional gets the size of the list from the struct :D
	 if (hashTable.hashTable[i]->next != NULL) {
	    listSize = hashTable.hashTable[i]->next->listSize;
	 } else {
	    listSize = 0;
	 }

	 entryList = getList(hashTable.hashTable[i], listSize);  //gets the array from hash index

	 //for loop for copying the array into the new array
	 for (int j = 0; j < listSize; j++) {
	    temp[temp_index] = entryList[j]; //copy multiple list in one array	
	    temp_index++;
	 }
	 free (entryList); //free the temperary array grabber
     }
     return (temp);
}


/*		getList
 * Takes an argument of a hashTable entry  
 * Returns an array of struct node of that specific entry
 * [This is part of the sortArray function]
 */

struct node* getList(struct node* sent, int listLength) {
     struct node* prev = sent;
     struct node* curr = sent -> next;
     //int listLength = listCount(sent);

     struct node* list = malloc(sizeof(struct node)*listLength);   
     int i = 0;
     //copies linked list into an array
     while (curr != NULL) {
	list[i].pair_of_words = curr -> pair_of_words;
        list[i].count = curr -> count;
	prev = prev -> next;
	curr = curr -> next;
	i++;	
     }
     return (list);
}

/*		newHashTable
 * This function takes in the old hashTable and the old size
 * It grows the size by a factor of 3  
 */

hashArray newHashTable(hashArray old_hashTable, FILE *fp) {
     //declare new hash struct
     hashArray biggerhashTable; 
     
     int newSize = old_hashTable.hashSize * 3; //increase hashtable by factor of 3

     biggerhashTable = init(newSize); //initialize new hashTable struct 
	     
     biggerhashTable.total_pairs = old_hashTable.total_pairs; //copy total pairs
							  
     //iterate through each table and rehash
     for (int i = 0; i < old_hashTable.hashSize; i++) {
	   copyHashIndex(biggerhashTable, old_hashTable.hashTable[i], true);
	}	    
		
     hashFree(old_hashTable); //go through freeing process for old hashTable

     biggerhashTable = buildTable(fp, biggerhashTable); //continue building table from file after copying from old hashTable
     return (biggerhashTable); //return the new hashTable 
}

/*              copyHashIndex
 * Loops through a specific index in the old hashTable array and sends to "biggerhashTable"
 */

void copyHashIndex (hashArray biggerhashTable, struct node* oldSent, bool isCopy) {
     struct node* prev = oldSent;
     struct node* curr = oldSent -> next;
     while (curr != NULL) {
	//calling listAdd with new info for the new hashTable
	int hashIndex = crc64(curr->pair_of_words) % biggerhashTable.hashSize;
	listAdd(biggerhashTable.hashTable[hashIndex], curr->pair_of_words, isCopy, curr->count);
	
	curr = curr -> next;
     }
     return;
}
/*		hashFree
 *  Frees the hash table and the linked list inside each entree
 *
 */
void hashFree(hashArray hashTable) {
     for (int i = 0; i < hashTable.hashSize; i++) {
    	LL_free(hashTable.hashTable[i]);
     }
     free(hashTable.hashTable);  //after freeing each entree, free the hashTable itself

}
/*		LL_free
 * Takes in an argument of a hashTable key and frees all allocated memory in that specific entry
 */
void LL_free(struct node *sent) {
     //temperary variables 
     struct node *prev = sent;
     struct node *curr = sent -> next;

     while (curr != NULL) {
        free(prev -> pair_of_words);
        free(prev);

        //traverse down
        prev = curr;
        curr = curr -> next;
     }
     //free final node
     free(prev -> pair_of_words);
     free(prev);
}

/*		listPrint
 * Prints the sorted array of structs
 */
void listPrint(struct node sortedArray[], int printLength) {
     for (int index = 0; index < printLength; index++){	   
        printf("%10d %s\n", sortedArray[index].count, sortedArray[index].pair_of_words);
     }
     return;
}
