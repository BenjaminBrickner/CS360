#include <stdio.h>
#include <errno.h>
#include <string.h>
int readable(char* inputPath);
int main(int argc, char** argv) {
	
	int result = readable(argv[1]);
	if (result < 0) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return -1;
	}
	printf("Result = %d\n", result);
	return 0;
}
