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
#define USERMAX 5
#define PONGGAME "pong_game"

//마피아 게임 변수들
char *START_STRING = "방에 입장하였습니다. 게임 대기중입니다..\n";
char *start_command1 = "start";
char *start_command2 = "\nGAME START\n";
char *mfmsg = "당신은 마피아입니다.\n";
char *plcmsg = "당신은 경찰입니다. 밤에 마피아가 누군지 알수 있는 기회가 부여됩니다.\n";
char *ctzmsg = "당신은 시민입니다. 마피아를 찾아주세요\n";
char *chating = "투표 전에 참여자들과 이야기를 나누세요\n";
char *morning = "\n\t낮이 밝았습니다. 마피아가 누구일지 결정해주세요\n";
char *nightmsg = "\t누구를 죽일 것인지 정해주세요: ";
char *vote = "------------------------------------------------------\n\t\t투표를 시작합니다.\n\t마피아라고 생각하는 사용자의 번호를 입력하세요\n";
char *voteresult = "------------------------------------------------------\n\t가장 많은 투표를 받은 사람은";
char *votenotice = "다음 사용자들을 제외하고 투표하세요)";
char *userdie = "\t사용자는 죽었습니다..투표에서 0을 눌러주세요\n";
char *victory = "\t그는 마피아입니다.\n\n";
char *nvictory = "\t그는 마피아가 아닙니다.\n";
char *mafiavictory = "\t★★★마피아의 승리입니다★★★\n";
char *ctzvictory = "\t★★★시민의 승리입니다★★★\n";
