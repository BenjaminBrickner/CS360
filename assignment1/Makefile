wordPair: main.o crc64.o getWord.o hashTable.o
	cc -o wordpairs -g main.o crc64.o getWord.o hashTable.o

main.o: main.c
	cc -g -c main.c
crc64.o: crc64.c
	cc -g -c crc64.c
getWord.o: getWord.c
	cc -g -c getWord.c
hashTable.o: hashTable.c
	cc -g -c hashTable.c
clean:
	rm main.o crc64.o getWord.o hashTable.o
