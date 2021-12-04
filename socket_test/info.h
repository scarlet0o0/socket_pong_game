#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <ncursesw/curses.h>
#include <locale.h>
#include <pthread.h>//쓰레드 사용을 위한 

#define PORTNUM 10003
#define SERVERIPADDR "10.0.2.15"
#define BUFLEN 256
#define TRUE 1
#define FALSE 0
#define QUITMSG "quit"
#define LISTENLEN 5
#define USERMAX 2
#define PONGGAME "pong_game"


