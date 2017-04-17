#ifndef _SY_SERVER_H
#define _SY_SERVER_H

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <sys/sendfile.h>
#include <sys/un.h>


typedef struct {  
    uint16_t port;
    struct in_addr ipaddress; 
    int timeout;
}sys_config_t;



int sy_config(void);
void sy_init(void);
void sy_listen(void);
void sy_accept(void);




#endif