#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "assignment7.h"

#define FILES 2
#define THREADS 10

#define ERROR(f, v) { if (f == v) { fprintf(stderr, "%m\n"); return errno; } }

char * const files[FILES] = {
	"lorem",
	"webster_random"
};

int main(int argc, char * argv[]) {
	for (size_t f = 1; f < FILES; f++) {
		printf("Testing %s...\n", files[f]);

		// Open file
		int fd = open(files[f], O_RDONLY);
		ERROR(fd, -1)
		// Allocate memory for file
		off_t eof = lseek(fd, 0, SEEK_END);
		ERROR(eof, -1)
		char * data = calloc(eof, sizeof(off_t));
		ERROR(data, NULL)
		ERROR(lseek(fd, 0, SEEK_SET), -1)
		// Read file into memory
		ERROR(read(fd, data, eof), -1)
		// Find number of newlines
		unsigned int newlines = 0;
		for (size_t c = 0; c < eof; c++)
			if (data[c] == '\n')
				newlines++;
		// For each line, get the address of its starting byte
		char * lines[newlines + 1], * temp_lines[newlines + 1];
		for (size_t line = 0, c = 0, start; line <= newlines; line++) {
			for (start = c; data[c] != '\0' && data[c] != '\n'; c++);
			lines[line] = strndup(&data[start], c++ - start + 1);
			ERROR(lines[line], NULL)
		}
		// Free original data chunk from memory and close file
		free(data);
		ERROR(close(fd), -1)

		struct timespec before, after;
		for (unsigned t = 1; t < THREADS; t++) {
			setSortThreads(t + 1);
			ERROR(clock_gettime(CLOCK_REALTIME, &before), -1)
			for (int i = 0; i < 5000; i++) {
				// Duplicate array (so that each call is given the data in the original unsorted order)
				for (size_t line = 0; line <= newlines; line++)
					temp_lines[line] = lines[line];
				sortThreaded(temp_lines, newlines + 1);
			}
			ERROR(clock_gettime(CLOCK_REALTIME, &after), -1)
			printf("\t With %2u threads, it took %3lu.%ld seconds.\n", t + 1, after.tv_nsec < before.tv_nsec ? after.tv_sec - before.tv_sec - 1 : after.tv_sec - before.tv_sec, after.tv_nsec < before.tv_nsec ? after.tv_nsec - before.tv_nsec + 1000000000L : after.tv_nsec - before.tv_nsec);
		}

		// Free string data
		for (size_t line = 0; line <= newlines; line++)
			free(lines[line]);
	}
	return 0;
}
