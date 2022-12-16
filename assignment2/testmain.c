#include "headers.h"

//testing lineNum
int main() {
	char* word = "a b c";
	char* dictionary = "asdfaasfasf";
	int line_number = lineNum(dictionary, word, 9);

	printf("%s is located at line %d\n", word, line_number);

	return 0;
}
