/*
 * 	INSERT SERVER CODE HERE
 */
#include "myftp.h" 
#define PORT_NUMBER 50000
/*
*           My Main Loop for commands
*/
void listenLoop(int clientID, int clientFD) {
    int isDataOn = 0;
    char CDbuf[PATH_MAX];
    char clientCmd[PATH_MAX];
    char buff[1];
    int n, dataListen, err, i;
    int x, *port = &x; //Note: this is a pointer to be able to access it without making globals
    int y, *dataSocket = &y;
    
    printf("Client %d has connected\n", clientID);
    fflush(stdout);
    while (1) {

        //keep reading input until reaches new line
        for (i = 0; i < PATH_MAX; i++) {
            if (read(clientFD, buff, 1) == 0) {
                printf("Client %d has disconnected\n", clientID);
                return;
            }
            if (buff[0] == '\n') {
                break;
            } else {
                clientCmd[i] = buff[0];
            }
        }
        if (i == PATH_MAX) {
            printf("Client reached limit of command request\n");
            return;
        }
        if (n < 0 ) {
            printf("reading from client ran into an error\n");
            write(clientFD, "E", 1);
            write(clientFD, strerror(errno), strlen(strerror(errno)));
            write(clientFD, "\n", 1);
            return;
        }
        clientCmd[i] = '\0';

    /*       =====  CONTROL CENTER =====   
    */

        //establish a data connection between server and client
        if (clientCmd[0] == 'D') {
            if (isDataOn) {
                fprintf(stderr, "Client sent another data socket request. Not allowed\n");
            } else {
                dataListen = makeDataConnection(clientFD, dataSocket, port);
                //error check
                if (dataListen == -1) {
                    fprintf(stderr, "makeDataConnection ran into an error\n");
                } else {
                    isDataOn = 1; //set flag on 
            }
            }
        }

        //client sent "rls" request (ls would be in the client side)
        else if (clientCmd[0] == 'L') {   
            if (isDataOn) {
                printf("Client %d used remote list in %s\n", clientID, getcwd(CDbuf, PATH_MAX));
                list(clientFD, dataListen);
            } else {
                fprintf(stderr, "The data connection wasn't made\n");
            }
            //close data connection socket
            close(*dataSocket);
            close(dataListen);
            isDataOn = 0; //set flag to false after closing data socket 
        }
            
        //client requesting for a File
        else if (clientCmd[0] == 'G') {
            //send file to client 
            if (isDataOn) {
                if (getFile(clientFD, dataListen, clientCmd+1) == 0) {
                    printf("Client %d has retrieved file: %s\n", clientID, clientCmd+1);
                    fflush(stdout);
                }
                close(*dataSocket);
                close(dataListen);
                isDataOn = 0;
            } else {
                fprintf(stderr, "The data connection wasn't made\n");
            }
        }
        else if (clientCmd[0] == 'P') {
            if (isDataOn) {
                if (putFile(clientFD, dataListen, clientCmd+1) == 0) {
                    printf("Client %d has sent file: %s\n", clientID, clientCmd+1);
                    fflush(stdout);
                } else {
                    printf("putFile ran into an error\n");
                    fflush(stdout);
                }
                close(*dataSocket);
                close(dataListen);
                isDataOn = 0;                
            } else {
                fprintf(stderr, "The data connection wasn't made\n");
            }
        }
        //Change Directory request
        else if (clientCmd[0] == 'C') {
            if (rcd(clientFD, clientCmd+1) == 0) {
                printf("Client %d has changed remote path to %s\n", clientID, getcwd(CDbuf, PATH_MAX)); 
                fflush(stdout);
            }
        }
        
        
        //client entered "exit" and this loop function exits 
        else if (clientCmd[0] == 'Q') {
            printf("Client %d has disconnected\n", clientID);
            fflush(stdout);
            write(clientFD, "A\n", 2); 
            return;
        }
        else {
            fprintf(stderr, "The client's command is invalid\n");
        }
    }
}

/*              My Main Function
*   Creates the socket for clients to join
*/

void main(int argc, char** argv) {
    int socketFD, listenFD, fork_ret; 
    struct sockaddr_in servAddr;
    struct sockaddr_in clientAddr;
    socklen_t len = sizeof(struct sockaddr_in);

    memset(&servAddr, 0, sizeof(servAddr)); //clears address structure 
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = ntohs(PORT_NUMBER); //give it a port number specified 
    servAddr.sin_addr.s_addr = htons(INADDR_ANY);
    //make a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        fprintf(stderr, "(main) making socket error\n");
        perror("Error");
        exit(1);
    }

    //clears socket and make it available after server shuts down
    setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)); 

    //bind socket
    if (bind(socketFD, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
        fprintf(stderr, "(main) Bind error\n");
        perror("Error");
        exit(1);
    }

    //set socket name
    if (getsockname(socketFD, (struct sockaddr *) &servAddr, &len) < 0) {
        printf("(main) get socket name error\n");
        perror("Error");
        exit(1);
    }
    /* Note: Upon return from getsockname(), servaddr.sin_port will contain the port number assigned by bind().*/
    int newPort = ntohs(servAddr.sin_port); //this command puts the bytes of the port in order 
    //this port is what the client needs to connect
    printf("Your port number is %d\n", newPort);
    //listen for socket (doesn't actually listen) and accepts 4 connections at a time 
    if (listen(socketFD, 4) < 0) {
        printf("(main) listen error\n");
        perror("Error");
        exit(1);
    }
    

    //infinite loop for clients --- forks processes for each client connected
    while (1) {
        //opens connection for client to join
        listenFD = accept(socketFD, (struct sockaddr *) &clientAddr, &len);
        if (listenFD < 0) {
            printf("listenFD inside main rain into an error\n");
            perror("Error");
        } else {
            //if no error from accept, clone the process for the client
            
            if (fork_ret = fork() > 0) {
                waitpid(getpid(), NULL, WNOHANG); //cleans up specific child
                close(listenFD);        //close socket connection when done with that specific socket
            } else if (fork_ret == 0) {

                //in child, client run infinite read/write command loop until client disconnects
                listenLoop(getpid(), listenFD);
            } else {
                //fork returned a negative value (error)
                printf("initial client fork error\n");
                perror("Error");
                close(socketFD);
                close(listenFD);
                exit(1);
            }
        }
    }
}

/*                           makeDataConnection
*   This function makes a temperary data connection after receiving "D" from the control 
*   It makes a new socket (with setup) and returns the new socket file descriptor OR -1 if it ran into an error
*/
int makeDataConnection(int clientFD, int *dataSocket, int *port) {
    int socketFD, listenFD;
    struct sockaddr_in servAddr;
    struct sockaddr_in clientAddr;

    int len = sizeof(struct sockaddr_in);
    memset(&servAddr, 0, sizeof(servAddr)); 
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(0); //the 0 port number indicates to find ANY available port to connect
    servAddr.sin_addr.s_addr = htons(INADDR_ANY);

    //make a socket and save it 
    *dataSocket = socket(AF_INET, SOCK_STREAM, 0);
    fflush(stdout);
    if (*dataSocket < 0) {
        perror("Data Socket Error");
        write(clientFD, "E", 1);
        write(clientFD, strerror(errno), strlen(strerror(errno)));
        write(clientFD, "\n", 1);
        return -1;
    }

    //clears socket and make it available after server shuts down
    setsockopt(*dataSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)); 

    //bind socket
    if (bind(*dataSocket, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
        perror("Data Bind Error");
        write(clientFD, "E", 1);
        write(clientFD, strerror(errno), strlen(strerror(errno)));
        write(clientFD, "\n", 1);
        return -1;
    }

    //set socket name
    if (getsockname(*dataSocket, (struct sockaddr *) &servAddr, &len) < 0) {
        perror("Data GetSockName Error");
        write(clientFD, "E", 1);
        write(clientFD, strerror(errno), strlen(strerror(errno)));
        write(clientFD, "\n", 1);
        return -1 ;
    }
    /* Note: Upon return from getsockname(), servaddr.sin_port will contain the port number assigned by bind().*/

    //listen for socket (doesn't actually listen) and accepts 4 connections in queue
    if (listen(*dataSocket, 1) < 0) {
        perror("Data Listen Error");
        write(clientFD, "E", 1);
        write(clientFD, strerror(errno), strlen(strerror(errno)));
        write(clientFD, "\n", 1);
        return -1 ;
    }

    *port = ntohs(servAddr.sin_port);
    char portStr[10];
    sprintf(portStr, "%d", *port); //saves port number as a string
    //send acknowledgement to client 
    write(clientFD, "A", 1);
    write(clientFD, portStr, strlen(portStr));
    write(clientFD, "\n", 1);

    //accepts data connection from client 
    listenFD = accept(*dataSocket, (struct sockaddr *) &clientAddr, &len);
    if (listenFD < 0) {
        perror("Data Connection Error");
        write(clientFD, "E", 1);
        write(clientFD, strerror(errno), strlen(strerror(errno)));
        write(clientFD, "\n", 1);
        return -1;
    }

    //After getting both data sockets connected. Return the socket file descriptor
    return listenFD;

}
/*              List Function (server side)
*      This executes when the client wants the list of remote directory (rls)
*/
void list(int clientFD, int dataListen) {
    write(clientFD, "A\n", 2);
    /* ls -l , while the client does more -20 */
    if (fork()) {
        wait(NULL);
    } else {
        close(1); //close stdout
        dup(dataListen); //puts data FD at stdout (lowest available file descriptor)
        fflush(stdout);
        execlp("ls", "ls", "-l",  NULL);
        perror("Error"); //print error but don't exit
    } 
    //close(dataListen); //close after done
}

/*              Remote CD Function
*     This changes the directory on the server side 
*/

int rcd(int client, char* path) {
    if (access(path, X_OK) < 0) {
        write(client, "E", 1);
        write(client, strerror(errno), strlen(strerror(errno)));
        write(client, "\n", 1);
        return -1;
    }
    if (chdir(path) < 0) {
        write(client, "E", 1);
        write(client, strerror(errno), strlen(strerror(errno)));
        write(client, "\n", 1);
        return -1;
    }
    write(client, "A\n", 2);
    return 0;
}

/*              Get File Function
*      Copies the file to the client
*/

int getFile(int clientFD, int dataListen, char* file) {
    int n;
    char buff[120];
    struct stat fill, *info = &fill;

    //open file 
    int fd = open(file, 0, 00754);
    if (fd < 0) {
        write(clientFD, "E", 1);
        write(clientFD, strerror(errno), strlen(strerror(errno)));
        write(clientFD, "\n", 1);
        return -1;
    }
    //get the info of the file
    if (lstat(file, info) < 0) {
        write(clientFD, "E", 1);
        write(clientFD, strerror(errno), strlen(strerror(errno)));
        write(clientFD, "\n", 1);
        return -1;
    }
    //check if it's a regular file
    if (!(info->st_mode & S_IFREG)) {
        write(clientFD, "E", 1);
        write(clientFD, "This is not a regular file!\n", 28);
        return -1;
    }
    
    //after error checking, confirm to client 
    write(clientFD, "A\n", 2);
    
    //send file bytes to client through data connection
    while ((n = read(fd, buff, 120)) > 0) write(dataListen, buff, n);
    if (n < 0) {
        perror("Error in getFile");
        return -1;
    }
    return 0;

}

/*              Put File Function
*       Copies the file from the client to the server
*/

int putFile(int client, int dataListen, char* file) {
    //make a file
    int n;
    char buff[120];
    int fd = open(file, O_CREAT | O_EXCL | O_RDWR, 00754); //make a file with permission 754
    if (fd < 0) {
        write(client, "E", 1);
        write(client, strerror(errno), strlen(strerror(errno)));
        write(client, "\n", 1);
        return -1;
    }

    write(client, "A\n", 2);
    while ((n = read(dataListen, buff, 120)) > 0) write(fd, buff, n);
    if (n < 0) {
        perror("Error when reading file in getFileClient");
        return -1;
    }
    return 0;
}
/*                      Show File
*   Displays the contents of the file and sends it to client via data socket
*/
void showFile(int clientFD, int dataListen, char* file) {
    struct stat area, *s = &area;

    //getting info of the file
    if (lstat(file, s) != 0) {
        write(clientFD, "E", 1);
        write(clientFD, strerror(errno), strlen(strerror(errno)));
        write(clientFD, "\n", 1);
        return;
    }
    //check if it's regular
    if (!(s->st_mode & S_IFREG)) {
        write(clientFD, "E", 1);
        write(clientFD, "The file is not regular.\n", 25);
        return;
    }
    //check access and is readable
    if ((access(file, F_OK | R_OK)) != 0) {
        write(clientFD, "E", 1);
        write(clientFD, strerror(errno), strlen(strerror(errno)));
        write(clientFD, "\n", 1);
        return;
    } 
    //send contents of the file to the client
    if (fork()){
        wait(NULL);
    } else {
        close(1); //close stdout
        dup(dataListen); //puts data FD at stdout (lowest available file descriptor)
        fflush(stdout);
        execlp("cat", "cat", file,  NULL);
        perror("Error"); //print error but don't exit
    }

}


