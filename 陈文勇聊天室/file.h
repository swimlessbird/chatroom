
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <netdb.h>         
#include <string.h>         
#include <stdlib.h>        
#include <unistd.h>         
#include <signal.h>  
#include <pthread.h>       
#include <sys/ipc.h>         
#include <sys/shm.h>         
#include <sys/sem.h>
#include <sys/msg.h>
//#include <sqlite3.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

struct online
{
	int fd;
	char name[20];
	                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
	struct online *next;
};

typedef struct online Online;
