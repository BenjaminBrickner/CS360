//Network Libraries
#include <sys/un.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

//misc libraries
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

//server functions 
void listen_loop();
void list(int clientFD, int dataListen);
int makeDataConnection(int clientID, int *dataSocket, int *port);
int rcd(int client, char* path);
int getFile(int clientFD, int dataListen, char *file);
int putFile(int clientFD, int dataListen, char *file);
void showFile(int clientFD, int dataSocket, char* file);



//client functions
void locallist();
void getFileClient(int socketID, char* addr, char* path);
void putFileClient(int socketID, char* addr, char* path);
int newDataSocket(int SocketID, char* servAddr);
void RLSFunction(int socketID, char* addr);
void changDir(char* path);
void serverChangDir(int socketID, char* path);
void showFileClient(int socketID, char* addr, char* file);
