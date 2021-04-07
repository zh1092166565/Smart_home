#ifndef _MAKEU_NET_H_
#define _MAKEU_NET_H_


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>


#define QUIT_STR "quit"
#define LED_ON	1
#define LED_OFF	2
#define BUZER_ON	3
#define BUZER_OFF	4
#define GYR_ON	5
#define GYR_OFF	6
#define CAMERA 7




#endif

