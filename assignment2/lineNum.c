#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


int lineNum(char* dictionaryName, char* word, int dictWidth) {
	char buff[dictWidth];
	int fd;
	int low, mid, high;

	fd = open(dictionaryName, O_RDONLY, 0); //opens file for read only
	if (fd == -1) {
		int i = 0;
	        while (strerror(errno)[i] != '\0') {
			i++;
		}	
		write(STDERR_FILENO,strerror(errno), i);
		return (errno);
	}
	//gets high and low values from lseek
	high = lseek(fd, 0, SEEK_END)/dictWidth - 1;
	low = lseek(fd, 0, SEEK_SET);
	
	int index = 0;
	char parsedWord[dictWidth];

	//copy letters
	for (index = 0; index < dictWidth; index++) {
		if (word[index] == '\0') break;
		parsedWord[index] = word[index];
	}
	//add spaces and null terminator at end
	for (int i = index; i < dictWidth; i++) {
		parsedWord[i] = ' ';
	}
	parsedWord[dictWidth-1] = '\0';

	//binary search algorithm
	int prev = 0;
	int curr = high * dictWidth;
	while (1==1) {
		mid = (high+low)/2; //mid of line number
		
		curr = lseek(fd, mid * dictWidth, SEEK_SET); //sets the pointer to the beginning of word	
		read(fd, buff, dictWidth);  //reads and saves it in buff
 
		buff[dictWidth-1] = '\0'; //add null terminator at end

		//if checking same place, exit
		if (prev == curr) {
                        close(fd);
                        return (-1 * (mid+1));
                }
                prev = curr;

		if (strcmp(buff, parsedWord) > 0) high = mid - 1;
		if (strcmp(buff, parsedWord) < 0) low = mid + 1;

		if (strcmp(buff, parsedWord) == 0) {
			close(fd);
			return (mid+1);
		}
	}	
}
