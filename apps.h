#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>

#define ADDRESS "tejo.tecnico.ulisboa.pt"
#define PORT "58011"
#define DOTDEC_IP 128
#define PASSWORD_STORE 9
#define MAX_PASS 8
#define USER_STORE 6
#define MAX_ID 5
#define MAX_VERIFICATION_NUMBERS 5
#define MAX_RANDOM 4
#define MAX_COMMAND 3
#define MAX_MESS 100
#define MAX_BUFFER 1024
#define MAX_FNAME 30
#define BOOLEAN 6
#define max(A,B) ((A)>=(B)?(A):(B))

/*--------Client------------*/

int fdClient,errcode;
ssize_t nClient;
socklen_t addrClientlen;
struct addrinfo hintsClient,*resClient;
struct sockaddr_in addrClient;
char bufferClient[MAX_BUFFER];

/*--------Client(for User only)------------*/

int fdClient2,errcode2;
ssize_t nClient2;
socklen_t addrClientlen2;
struct addrinfo hintsClient2,*resClient2;
struct sockaddr_in addrClient2;
char bufferClient2[MAX_BUFFER];

/*--------Server------------*/

int fdServer;
ssize_t nServer,nreadServer;
socklen_t addrServerlen;
struct addrinfo hintsServer,*resServer;
struct sockaddr_in addrServer;
char bufferServer[MAX_BUFFER];



fd_set rfds;

