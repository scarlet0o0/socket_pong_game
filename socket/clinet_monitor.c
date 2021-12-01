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
#include "info.h"
int main(int argc, char*argv[]){
	char socketname[20] = "client_socket_";
	char buf[BUFLEN];
	struct sockaddr_un server,clinet;
	int access_sock, comm_sock, len;
	
	strcat(socketname,argv[1]);
	unlink(socketname);
	if((access_sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
		perror("socket"); exit(1);
	}
	
	memset((char*)&server, '\0',sizeof(server));
	server.sun_family = AF_UNIX;
	strcpy(server.sun_path,socketname);
	printf("UNIX 소켓파일명:%s\n",socketname);
	if(bind(access_sock,(struct sockaddr*)&server,sizeof(server))){
			perror("bind");exit(1);
	
	}
	if(listen(access_sock,LISTENLEN)){
		exit(1);
	}
	

	//서버와 연결위한 설정
	int sock1;
	char buf1[BUFLEN];
	struct sockaddr_in server1;

	if((sock1 = socket(AF_INET,SOCK_STREAM,0)) == -1){
		perror("socket");exit(1);
	}

	memset((char*)&server1,'\n',sizeof(server1));
	server1.sin_family = AF_INET;
	server1.sin_addr.s_addr = inet_addr(SERVERIPADDR);
	server1.sin_port = htons(PORTNUM);


	while(TRUE){
		printf("[info] Unix socket : 클라이언트 접속 대기 중...\n");
		len = sizeof(clinet);
		if((comm_sock = accept(access_sock,(struct sockaddr*)&clinet,&len))==-1){
			perror("accept"); break;
		}
		printf("[info] Unix socket : 클라이언트 접속 확인\n");

		int flags = fcntl(comm_sock,F_GETFL,0);
		fcntl(comm_sock,F_SETFL,flags|O_NONBLOCK);
		memset(buf, '\0' ,BUFLEN);

		//서버로 연결신호를 보낸다
		if(connect(sock1,(struct sockaddr *)&server1,sizeof(server1))){
			perror("server");exit(1);
		}
		printf("[info] Inet socket : 서버 접속 완료\n");
		int flags1 = fcntl(sock1,F_GETFL,0);
		fcntl(sock1,F_SETFL,flags1|O_NONBLOCK);

		//클라이언트 인풋과 메시지를 주고 받는다.
		while(TRUE){
			
			if(recv(comm_sock,buf,sizeof(buf),0) >0){
				printf("[ME]:%s",buf);
				if(send(sock1,buf,sizeof(buf),0) == -1){
					perror("send");break;
				}
			}
			if(recv(sock1,buf1,BUFLEN,0) >0){
				if(strncmp(buf1,QUITMSG,strlen(QUITMSG))==0){
					printf("[INFO]소켓을 닫고 프로그램을 종료합니다\n");
					break;
				}
				printf("%s",buf1);

			}
			
			memset(buf,'\0',BUFLEN);
			usleep(50);
		}
		close(comm_sock);
		close(sock1);
		break;
	}
	close(access_sock);
	return 0;
}



