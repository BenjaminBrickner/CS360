//Network Libraries
#include <sys/un.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
//Misc Libraries
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>


#define PORT_NUMBER 49999

void server();
void client(char *);
void error_function();

int main(int argc, char const *argv[]){
    if (argc > 3 || argc == 1) {
        fprintf(stderr, "Usage: ./assignment8 client <address> OR ./assignment8 server\n");
        exit (-1);
    }
    if (strcmp(argv[1], "server") == 0) {

        //checking if two arguments were used for picking server
        if (argc == 3) {
            fprintf(stderr, "Usage: ./assignment8 client <address> OR ./assignment8 server\n");
            exit (-1);
        }
        //call server function 
        server();
    } else if (strcmp(argv[1], "client") == 0) {
        client((char *) argv[2]);
    } else {
        fprintf(stderr, "Usage: ./assignment8 client <address> OR ./assignment8 server\n");
        exit (-1);
    }
    return 0;
}

/*              The Server Function
*       It's primary goal is to represent the server side of the program
*   It will wait for a connection, and fork if it receives a connection 
*/
void server() {
    int client_count = 0;
    int socketFD, listenFD, hostentry;
    struct sockaddr_in servAddr;
    struct sockaddr_in clientAddr;
    char hostName[NI_MAXHOST];

    //set the socket address structure
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT_NUMBER);
    servAddr.sin_addr.s_addr = htons(INADDR_ANY);

    socketFD = socket(AF_INET, SOCK_STREAM, 0); //make a socket end point
    if (socketFD < 0) error_function(); //error check and exit
    setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)); //clears socket and make it available is server is killed :) 
    
    //binds the socket to the port (make sure to error check this)
    if(bind(socketFD, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) error_function();

    if(listen(socketFD, 1) < 0) error_function();

    int len = sizeof(struct sockaddr_in);

    int fork_ret;
    while (1) {
        listenFD = accept(socketFD, (struct sockaddr *) &clientAddr, &len);
        if (listenFD < 0) error_function();
            
        client_count++;

        if (fork_ret = fork() > 0) { //parent
            wait(NULL);
        } else if (fork_ret == 0) { //child
            hostentry = getnameinfo((struct sockaddr *) &clientAddr, sizeof(clientAddr), hostName, sizeof(hostName), NULL, 0, NI_NUMERICSERV);
        
            //print client name and counter
            printf("%s %d\n", hostName, client_count);
 	    fflush(stdout);           
            //grab date and write first 18 bytes of the date output to the client
            time_t curr_time;
            if (time(&curr_time) < 0) error_function();
            write(listenFD, ctime(&curr_time), 19);
	    fflush(listenFD);
        } else { //error check
            error_function();
        }
        }
        return;
}

/*                        Client Function
*   This is the client function, representing the client when starting the program.   
*   It will write the date received from the server to stdout and must be exactly 18
*   bytes. 
*/
void client(char *address) {
    //quickly convert a number into a string or "getaddrinfo" argument
    char portStr[6];
    sprintf(portStr, "%d", PORT_NUMBER);

    struct addrinfo hints, *actualdata;
    int socketFD, listenFD;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
    memset(&hints, 0, sizeof(hints));
    int err = getaddrinfo(address, portStr, &hints, &actualdata);
    if (err != 0) {
        fprintf(stderr, "Error: %s\n", gai_strerror(err));
        exit(1);
    }
    socketFD = socket(actualdata -> ai_family, actualdata -> ai_socktype, 0);
    if (socketFD < 0) error_function();

    if (connect(socketFD, actualdata -> ai_addr, actualdata -> ai_addrlen) < 0) error_function();

    /* You are now connected to the server*/
    char dateOutput[19];
    read(socketFD, dateOutput, 19);
    dateOutput[18] = '\n';
    write(1, dateOutput, 19);
    fflush(stdout);
    return;
}


//just an error function for error checking, if and only if the error number is set 
void error_function() {
    perror("Error");
    exit (-1*errno);
}
