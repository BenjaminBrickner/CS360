/*
 * 	INSERT CLIENT CODE HERE
 */
#include "myftp.h"

void main(int argc, char *argv[]) {


    //argument check 
    //format -> ./myftp [port] [address/IP]
    if (argc > 4) {
        fprintf(stderr, "Too many arguments. Usage: ./myftp [port] [host/IP]\n");
        exit (0);
    } 
    if (argc < 3) {
        fprintf(stderr, "Not enough arguments. Usage: ./myftp [port] [host/IP]\n");
        exit(0);
    } 
    int socketID;
    struct addrinfo fill, *client;
    fill.ai_socktype = SOCK_STREAM;
    fill.ai_family = AF_INET;

    memset(&fill, 0, sizeof(fill));

    int err = getaddrinfo(argv[2], argv[1], &fill, &client); //argv[2] is address, argv[1] is the port
    if (err != 0) {
        fprintf(stderr, "Error: %s\n", gai_strerror(err));
        freeaddrinfo(client);
        exit(1);
    }

    socketID = socket(client -> ai_family, client -> ai_socktype, 0);
    if (socket < 0) {
        perror("Error");
        freeaddrinfo(client);
        exit (-1);
    }

    if (connect(socketID, client -> ai_addr, client -> ai_addrlen) < 0) {
        perror("Error");
        close(socketID);
        freeaddrinfo(client);
        exit (-1);
    }

    /*The main loop for commands*/

    int n, dataSocket;
    char servCtrlReply[2];
    char input[256];
    char *cmd;
    char *path;

    printf("> ");
    fflush(stdout);
    while (1) {
        n = read(0, input, 256);
        

        //if user entered only a new line
        if (n==1) {
            printf("Invalid command. Here are the commands.\n");
            printf("---------------------------------------------------------------------\n");
            printf("exit -- exits program\n");
            printf("cd <pathname> -- change local working directory\n");
            printf("rcd <pathname> -- change remote working directory\n");
            printf("ls -- lists contents in a local directory 20 lines at a time\n");
            printf("rls -- lists contents in a remote directory 20 lines at a time\n");
            printf("get <pathname> -- retreives <pathname> from the server to the client\n");
            printf("show <pathname> -- shows the content in the file 20 lines at a time\n");
            printf("put <pathname> -- transmits the contents of <pathname> to the server\n");
            fflush(stdout);
        } else 
        {
        
        //input[n] = '\0'; //add null byte at the end (read doesn't add the null byte)
            input[n-1] = '\0';  //NOTE: *(input+n-1) would also word :) 

            //parse the one input string into two catagories: command and path 
            cmd = strtok(input, " "); 
            path = strtok(NULL, " ");

            /*  The following if-elseif list does different task depending on the user's wishes */

            //checking if user entered nothing (unless it's just a new line)
            if (cmd == NULL) {
                printf("Invalid command\n");
                fflush(stdout);
            }

            /*  ls command*/
            else if (strcmp(cmd, "ls") == 0) {
                locallist();
            }

            /*  rls command */
            else if (strcmp(cmd, "rls") == 0) {
                RLSFunction(socketID, argv[2]);
            }                

            /*  cd command */
            else if (strcmp(cmd, "cd") == 0) {
                if (path == NULL) {
                    fprintf(stderr, "You did not specify the path\n");
                } else {
                    changDir(path);
                }
            }

            /*  rcd command */
            else if (strcmp(cmd, "rcd") == 0) {
                if (path == NULL) {
                    fprintf(stderr, "Need a path\n");
                } else {
                serverChangDir(socketID, path);
                }
            }

            /*  get command */
            else if (strcmp(cmd, "get") == 0) {
                if (path == NULL) {
                    fprintf(stderr, "Missing file argument\n");    
                } else {
                    getFileClient(socketID, argv[2], path);
                }
            }

            /*  show command */
            else if (strcmp(cmd, "show") == 0) {
                if (path == NULL) {
                    fprintf(stderr, "Missing file argument\n");  
                } else {
                    showFileClient(socketID, argv[2], path);
                }
            }

            /*  put command */
            else if (strcmp(cmd, "put") == 0) {
                if (path == NULL) {
                    fprintf(stderr, "Missing file argument\n");
                } else {
                putFileClient(socketID, argv[2], path);
                }
            }
            /*  exit command */
            else if (strcmp(cmd, "exit") == 0) {
                write(socketID, "Q\n", 2); //send server quit message 
                if (close(socketID) < 0) {
                    perror("Error when closing socket");
                }
                freeaddrinfo(client); //free addrinfo
                exit(0);  
            } 
            else {
                fprintf(stderr, "Invalid command\n");
            }
        }

    //prompting user again
    printf("> ");
    fflush(stdout);
    } //end of while loop
} //end of main 


/*                          Local List
*   Prints the list of local files 20 lines at at time 
*/
void locallist() {
    int fd[2];

    if (fork()) {
        wait(NULL);  //Wait for the ls command to finish in the child 
    } else {
        /* ls -l | more -20 */
        if (pipe(fd) < 0) {
            perror("Error");
            return;
        }
        if (!fork()) {
            /* Child Process {pipe write end} */

            //close connections and duplicate fd[1]
            close(1); //close stdout
            dup (fd[1]); //duplicate fd[1] and replaces stdout (lowest available file descriptor)
            close(fd[1]); //close fd[1] since it's duplicated
            close(fd[0]); //close fd[0], since the parent's printing to stdout

            //execute ls command 
            execlp("ls", "ls", "-l", NULL);
            perror("(LocalList) Error in Child"); //print error but don't exit
        } else {
            /*Parent Process {pipe read end} */

            close(0); //close stdin
            dup(fd[0]); //duplicate stdin and replace stdin (lowest available file descriptor)
            close(fd[0]); //close the duplicate of the stdin
            close(fd[1]); //close stdout pipe
            execlp("more", "more", "-20", NULL); //shows only first 20 lines of the ls -l command 
            perror("(LocalList) Error in Parent"); //error but no exit 
        }
    }
    
    return;
}

/*                  RLS Function
*   Gets the list of files/directories from server to client
*/
void RLSFunction(int socketID, char* addr) {
    int n;
    char servReply[2];
    char buff[120];

    //setup data connection
    int dataSocket = newDataSocket(socketID, addr);
    if (dataSocket == -1) return;

    if (write(socketID, "L\n", 2) < 0) {
        perror("Error when sending L");
        close(dataSocket);
        return;
    }
    if (read(socketID, servReply, 2) < 0) {
        perror("Error when reading A or E in RLSFunction");
        close(dataSocket);
        return;
    }
    if (servReply[0] != 'A') {
        fprintf(stderr, "Error responce after sending L to server in RLS function\n");
        close(dataSocket);
        return;
    }
    //runs the more command in client side
    if (fork()) {
        wait (NULL);
    } else {
        close(0); //close stdin
        dup(dataSocket); //duplicate stdin and replace stdin with the server data socket
        execlp("more", "more", "-20", NULL); //shows only first 20 lines of the ls -l command 
        perror("(RLSFunction) Error in Parent"); //error but no exit 
    }
    close(dataSocket);
}

/*              Change Directory
*   Changes current working directory on client side
*/
void changDir(char* path) {
    if (access(path, X_OK) != 0) {
        perror("Error");
        return;
    }
    int err = chdir(path);
    if (err < 0) {
        perror("Error");
        return;
    } else {
        printf("Success!\n");
    }
}
/*          Server Change Directory
*   Changes directory on the server side
*/
void serverChangDir(int socketID, char* path) {
    char servCtrlReply[MAX_INPUT];
    char error[MAX_INPUT];
    /*  send C and pathname to the server   */
    if (write(socketID, "C", 1) < 0) {
        perror("Error when sending C to the server");
        return;
    } 
    if (write(socketID, path, strlen(path)) < 0) {
         perror("Error when sending C path");
         return;
    }
    if (write(socketID, "\n", 1) < 0) {
        perror("Error when sending new line to server");
        return;
    }
    
    /*  get responce from server and see if it was successful for not*/
    if (read(socketID, servCtrlReply, MAX_INPUT) < 0) {
        perror("Error when getting reply after sending L");
    }
    if (servCtrlReply[0] == 'A') {
        printf("Success\n");
        fflush(stdout);
    } else {
        //loop until it reaches new line
        int i = 1;
        while(servCtrlReply[i] != '\n') {
            error[i-1] = servCtrlReply[i];
            i++;
        }
        error[i-1] = '\0';
        //print error
        printf("%s\n", error);
        fflush(stdout);
    }
}
/*              Show File Client
*   Gets the contents in a file from the server
*/
void showFileClient(int socketID, char* addr, char *file) {
    char servResponce[MAX_INPUT];
    int n;
    //if good, send the G <file>
    int dataSocket = newDataSocket(socketID, addr);
    if (dataSocket == -1) return;

    if (write(socketID, "G", 1) < 0) {
        perror("Error sending G in showFileClient");
        close(dataSocket);
        return;
    }
    if (write(socketID, file, strlen(file)) < 0) {
        perror("Error when sending file in showFileClient");
        close(dataSocket);
        return;
    }
    if (write(socketID, "\n", 1) < 0) {
        perror("Error sending new line in showFileClient");
        close(dataSocket);
        return;
    }
    //get answer from the server
    if ((n = read(socketID, servResponce, MAX_INPUT)) < 0) {
        perror("Error reading responce to server in showFileClient");
        close(dataSocket);
        return;
    }
    if (servResponce[0] == 'E') {
         char errorMsg[PATH_MAX];
        int i = 1;
        for (; i < PATH_MAX; i++) {
            errorMsg[i-1] = servResponce[i];
        }
        errorMsg[i] = '\0';
        fprintf(stderr, "Server responded with error: %s", errorMsg);
        close(dataSocket);
        return;
    }
    //runs the more command in client side
    if (fork()) {
        wait (NULL);
    } else {
        close(0); //close stdin
        dup(dataSocket); //duplicate stdin and replace stdin with the server data socket
        execlp("more", "more", "-20", NULL); //shows only first 20 lines of the ls -l command 
        perror("(LocalList) Error in Parent"); //error but no exit 
    }
    close(dataSocket);
}

/*              Get File Client
*   Retrieve a regular file from the server
*/
void getFileClient(int socketID, char* addr, char* file) {
    int n;
    char servResponce[MAX_INPUT];
    char buff[120];

    //make a new data socket
    int dataSocket = newDataSocket(socketID, addr);
    if (dataSocket == -1) return;

    //after data socket is established, request for a file from the server
    if (write(socketID, "G", 1) < 0) {
        perror("Error sending G in getFileClient");
        close(dataSocket);
        return;
    }
    if (write(socketID, file, strlen(file)) < 0) {
        perror("Error when sending file in getFileClient");
        close(dataSocket);
        return;
    }
    if (write(socketID, "\n", 1) < 0) {
        perror("Error sending new line in getFileClient");
        close(dataSocket);
        return;
    }

    //read acknowledgement from the server 
    if ((n = read(socketID, servResponce, MAX_INPUT)) < 0) {
        perror("Error reading responce to server in showFileClient");
        close(dataSocket);
        return;
    }
    servResponce[n] = '\0';

    //Server sent an error
    if (servResponce[0] == 'E') {
        char errorMsg[PATH_MAX];
        int i = 1;
        for (; i < PATH_MAX; i++) {
            errorMsg[i-1] = servResponce[i];
        }
        errorMsg[i] = '\0';
        fprintf(stderr, "Server responded with error: %s", errorMsg);
        return;
    }

    //make a file with the same name 
    int fd = open(file, O_CREAT | O_EXCL | O_RDWR, S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH); //make a file with permissions 741
    if (fd < 0) {
        perror("Error");
        close(dataSocket);
        return;
    }

    //read the file data from server and write it to the file made 
    while ((n = read(dataSocket, buff, 120)) > 0) write(fd, buff, n);
    if (n < 0) {
        perror("Error when reading file in getFileClient");
        close(dataSocket);
        return;
    }
    printf("File %s has been received!\n", file);
    fflush(stdout);
}

/*                  Put File Client     
*   Sends file data to the server via data socket
*/
void putFileClient(int socketID, char* addr, char* path) {
    char servResponce[PATH_MAX];
   
    int fd = open(path, O_RDWR, 0);
    
    //checks if file doesn't exist or another error
    if (fd < 0) {
        perror("Error");
        return;
    }

    //check if the file is regular
    struct stat area, *s = &area;
    //get info of the file
    if (lstat(path, s) < 0) {
        perror("Error");
        return;
    }
    if (!(s->st_mode & S_IFREG)) {
        fprintf(stderr, "This is not a regular file\n");
        return;
    }
    //request data socket in order to send the file data
    int putDataSocket = newDataSocket(socketID, addr);
    if (putDataSocket == -1) return;

    //send P<pathname> to the server to let the server know "hey, I'm sending a file"
    if (write(socketID, "P", 1) < 0) {
        perror("Error sending request to server");
        close(putDataSocket);
        return;
    }
    if (write(socketID, path, strlen(path)) < 0) {
        perror("Error sending request to server");
        close(putDataSocket);
        return;
    }
    if (write(socketID, "\n", 1) < 0) {
        perror("Error sending request to server");
        close(putDataSocket);
        return;
    }

    //get a responce from the server
    if (read(socketID, servResponce, PATH_MAX) < 0) {
        perror("Error receiving acknowledgement from server");
        close(putDataSocket);
        return;
    }
    //checking if the responce is an error
    if (servResponce[0] == 'E') {
        char errorMsg[PATH_MAX];
        int i = 1;
        for (; i < PATH_MAX; i++) {
            errorMsg[i-1] = servResponce[i];
        }
        errorMsg[i] = '\0';
        fprintf(stderr, "Server responded with error: %s", errorMsg);
        close(putDataSocket);
        return;
    } 

    //after that, it's ready to be sent
    int n;
    char buff[120];
    while ((n = read(fd, buff, 120)) > 0) write(putDataSocket, buff, n);
    if (n < 0) {
        perror("Error sending file to server in putFileClient");
        close(putDataSocket);
        return;
    }
    printf("File %s has been sent!\n", path);
    fflush(stdout);
    close(putDataSocket);

}

/*                New Data Socket                                      */
/*  This function makes a new data socket if necessary                 */
/*  It returns the data socket file descriptor for transfering answers */
int newDataSocket(int socketID, char *servAddr) {
    int i = 1;
    char servResponce[MAX_INPUT];
    char port[MAX_INPUT];

    //send D to server for data connection
    if (write(socketID, "D\n", 2) < 0) {
        perror("Error when sending D to server");
        return -1;
    }

    //receive if the server accepted or reject 
    int n = read(socketID, servResponce, MAX_INPUT);
    if (n < 0) {
        perror("Error when reading A or E");
        return -1;
    }

    //scan input until newline has reached
    while (servResponce[i] != '\n') {
        port[i-1] = servResponce[i];
        if (i == MAX_INPUT-1) {
            fprintf(stderr, "servResponce length too long\n");
            return -1;
        }
        i++;
    }
    port[i-1] = '\0';
    if (servResponce[0] != 'A') {
        fprintf(stderr, "Error responce in server in newDataFunction function: %s\n", port);
        return -1;
    }
    //set last bit null byte
    servResponce[n-1] = '\0';

    /*  Now, we setup the new socket!   */

    struct addrinfo x, *newData; //x is just a filler for the newData pointer
    x.ai_socktype = SOCK_STREAM;
    x.ai_family = AF_INET;
    

    memset(&x, 0, sizeof(x)); 

    //pass in the same address (localhost for example) and the new port sent by the server
    int err = getaddrinfo(servAddr, servResponce+1, &x, &newData); 
    if (err != 0) {
        fprintf(stderr, "Error in getaddrinfo data: %s\n", gai_strerror(err));
        return -1;
    }

    //make a socket 
    int newDataID = socket(newData -> ai_family, newData -> ai_socktype, 0);
    if (socket < 0) {
        printf("socket error\n");
        perror("Error");
        return -1;
    }

    //attempt to connect to the server awaiting for client connection 
    if (connect(newDataID, newData -> ai_addr, newData -> ai_addrlen) < 0) {
        printf("connect error\n");
        perror("Error");
        close(newDataID);
        return -1;
    }

    //returns the data socket to send data through 
    return (newDataID);
}