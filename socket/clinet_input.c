#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include "info.h"

int main(int argc, char *argv[]){
	char socketname[20] = "client_socket_";
	char buf[BUFLEN];
	struct sockaddr_un server;
	int sock;
	int waiting_room=TRUE; //클라이언트가 채팅방에 있는지 대기실에 있는지 확인하는 변수
	
	
	if((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
		perror("socket"); exit(1);
	}
	
	memset((char*)&server, '\0',sizeof(server));
	server.sun_family = AF_UNIX;

	strcat(socketname,argv[1]);
	strcpy(server.sun_path,socketname);
	printf("UNIX 소켓파일명:%s",socketname);


	if(connect(sock,(struct sockaddr*)&server,sizeof(server))){
			perror("connect");exit(1);
	}
	printf("[info] unix socket: 서버 접속 완료\n");
	

	memset(buf,'\0',BUFLEN);//버퍼를 초기화
	while(TRUE){

		memset(buf,'\0',BUFLEN);//버퍼를 초기화
		printf("Enter: ");
		fgets(buf,BUFLEN,stdin);
		if(send(sock,buf,strlen(buf)+1,0) == -1){ //서버로 메시지 보낸다
			perror("send"); break;
		}

		if(strncmp(&buf[0],"2",strlen("2")) == 0 && waiting_room == TRUE){
			w = 0;
		}
		if(strncmp(buf,QUITMSG,strlen(QUITMSG)) == 0 && waiting_room == FALSE){
			w = TRUE;
		}
		
		//3을 입력하고 사용자가 대기실에 있다면
		if(strncmp(&buf[0],"3",strlen("3")) == 0  && waiting_room == TRUE){//프로그램을 종료합니다. 
			printf("[INFO] 소켓을 닫고 프로그램을 종료합니다.");
			break;
		}
						
	}

	close(sock);
	return 0;	
		
}




