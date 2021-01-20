#ifndef __TCP__NET__SOCKET__H
#define __TCP__NET__SOCKET__H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <pthread.h>

extern void log_interface();
extern void *read_message();
#endif
