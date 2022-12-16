main: myftpserve.o myftp.o
	gcc -o myftpserve myftpserve.o
	gcc -o myftp myftp.o

myftpserve:	myftpserve.o
	gcc -o myftpserve myftpserve.o

myftpserve.o: myftpserve.c myftp.h
	gcc -c myftpserve.c

myftp: myftp.o
	gcc -o myftp myftp.o

myftp.o: myftp.c myftp.h
	gcc -c myftp.c

clean:
	rm -f myftpserve myftp myftpserve.o myftp.o 