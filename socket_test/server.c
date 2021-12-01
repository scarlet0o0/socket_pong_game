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

#include "info.h"
#define MENU "<MENU>\n1.채팅방 목록 보기\n2.채팅방 참여하기 (사용법: 2 <채팅방 번호>)\n3.프로그램 종료\n(0을 입력하면 메뉴가 다시 표시됩니다.)"
#define USERMAX 5
#define CHATNUM 4

//함수 선언
int maxArr(int *n, int size);
void delete(int *arr, int size, int index);
void *t_function(void *data);
void handler(int sig);
//체팅방 정보 구조체
struct ChatRoom{
	char room_name[20];
	int room_id;
	int user[USERMAX]; // 사용자 소켓 배열
	int user_num;
};
//채팅방에 입장할려는 유저 
struct UserSockets{
	int user_num; 
	int user[USERMAX];
};
//채팅방을 나가려는 유저
struct ChatInfo {
	struct ChatRoom room;
	struct UserSockets new_users;
	struct UserSockets returned_users;
};
	
int main(void){
	char buf[BUFLEN];
	struct sockaddr_in server, client;
	int access_sock, comm_sock[10], len;
	
	struct ChatInfo chatinfo[CHATNUM];
	pthread_t p_thread[CHATNUM];
	
	char *cname[]={"Chateoom-0","Chatroom-1","Chatroom-2","ponggame"};//채팅방 이름

	int ret;
	fd_set readfds;

	//시간 변수
	struct timeval t;
	t.tv_sec = 0;
	t.tv_usec = 500;

	signal(SIGINT,handler);

	//소켓 생성
	if((access_sock = socket(AF_INET,SOCK_STREAM,0)) == -1){
		perror("socket");exit(1);
	}
	
	//소켓 논블로킹
	int flags = fcntl(access_sock,F_GETFL,0);
	fcntl(access_sock,F_SETFL,flags|O_NONBLOCK);
	
	memset((char*)&server,'\0',sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
        server.sin_port = htons(PORTNUM);

	if(bind(access_sock,(struct sockaddr*)&server,sizeof(server))){
		perror("bind1"); exit(1);
	}
	if(listen(access_sock,LISTENLEN)){
		perror("listen"); exit(1);
	}
	
	//쓰레드 생성
	printf("[MAIN] 사용자별 숫자번호는 소켓 기술자 번호 입니다.\n");
	for(int i =0 ;i<CHATNUM;i++){//초기화 
		chatinfo[i].room.room_id=i;
		chatinfo[i].room.user_num = 0;
		chatinfo[i].new_users.user_num = 0;
		chatinfo[i].returned_users.user_num = 0;
		strcpy(chatinfo[i].room.room_name,cname[i]);
		pthread_create(&p_thread[i],NULL,t_function,(void*)&chatinfo[i]);
		printf("[Ch.%d] 채팅방을 오픈합니다.\n",i+1); 
	}
	
	//대기실
	int waiting_room_n = 0; // 대기실 인원수
	int chat_num = 0; //채팅방 총 인원수
	while(TRUE){
		
		
		//1.Nonblocking accept 호출
		len = sizeof(client);
		if((comm_sock[waiting_room_n] = accept(access_sock,(struct sockaddr *)&client,&len)) > 0 && waiting_room_n<10){

			//논블로킹설정
			flags = fcntl(comm_sock[waiting_room_n],F_GETFL,0);
			fcntl(comm_sock[waiting_room_n],F_SETFL,flags|O_NONBLOCK);

			printf("[MAIN] 새로운 사용자가 접속했습니다 : %d\n",comm_sock[waiting_room_n]);

			//메뉴를 클라이언트한테 보내준다.
			memset(buf,'\0',BUFLEN);
			strcpy(buf,MENU);
			if(send(comm_sock[waiting_room_n],buf,sizeof(buf),0) == -1){
				perror("send");
				exit(1);
			}

			waiting_room_n++; //대기실 인원수 증가
			chat_num++;      // 채팅방 총 인원수 증가

		}

		//2. 채팅방에 탈퇴한 사용자가 있는지 확인
		for(int i = 0;i<CHATNUM;i++){
			int cl =0;
			while(chatinfo[i].returned_users.user_num>0){
				comm_sock[waiting_room_n]=chatinfo[i].returned_users.user[cl];
				printf("[MAIN]채팅방 탈퇴 사용자 탐지:%d\n",comm_sock[waiting_room_n]);
				chatinfo[i].returned_users.user_num--;
				waiting_room_n++;
				cl++;
			}
		}
		
		//3. 대기실에 있는 클라이언트에 요청을 처리
		FD_ZERO(&readfds);for( int i = 0;i<waiting_room_n;i++) FD_SET(comm_sock[i],&readfds);
		ret = select(maxArr(comm_sock,waiting_room_n)+1,&readfds,NULL,NULL,&t);
		
		if(ret>0){
			int i =0;
			while(ret >0){
				if(FD_ISSET(comm_sock[i],&readfds)){
					memset(buf,'\0',BUFLEN);
					int menu_num = -1;
					int chat_id = -1;
					if(recv(comm_sock[i],buf,sizeof(buf),0) >0){
						printf("[MAIN]사용자%d 의 메시지 :%s\n",comm_sock[i], buf);
						if(send(comm_sock[i],buf,sizeof(buf),0) == -1)//사용자 메시지 사용자에게 다시 전달
						{
							perror("send");
							exit(1);
						}

						menu_num = buf[0]-'0';
						switch(menu_num){
							case 0://메뉴판 클라이언트 한테 전송
								strcpy(buf,MENU);
								if(send(comm_sock[i],buf,sizeof(buf),0) == -1){
									perror("send");
									exit(1);
								}
								break;
							case 1://채팅방의 정보를 클라이언트 한테 전송
								strcpy(buf,"<chatRoom info>\n");
								for(int j = 0; j < CHATNUM ; j++){
									char id[3],chat_num[3];
									sprintf(id,"%d",chatinfo[j].room.room_id);
									sprintf(chat_num,"%d",chatinfo[j].room.user_num);
									strcat(buf,"[ID: ");
									strcat(buf,id);
									strcat(buf,"]");
									strcat(buf,chatinfo[j].room.room_name);
									strcat(buf," (");
									strcat(buf,chat_num);
									strcat(buf,"/5)\n");
								}	
								if(send(comm_sock[i],buf,sizeof(buf),0) == -1){
									perror("send");
									exit(1);
								}


								break;
							case 2://채팅방의 참여
								chat_id =  buf[2] - '0';
								if(chat_id>=0 && chat_id<=CHATNUM-1){
									chatinfo[chat_id].new_users.user[chatinfo[chat_id].new_users.user_num] = comm_sock[i];
									printf("[MAIN] 사용자 %d가 채팅방 %d 에 참여합니다.\n",comm_sock[i],chat_id);
									chatinfo[chat_id].new_users.user_num++;
									delete(comm_sock,waiting_room_n,i);//배열에서 소켓 삭제
									waiting_room_n--;//대기실 인원수 한명 감소
								}
								else{
									strcpy(buf,"메뉴번호를 다시 입력헤주세요\n");
									if(send(comm_sock[i],buf,sizeof(buf),0) == -1){
										perror("send");
										exit(1);
									}

								}
								break;
							case 3://종료
								printf("%d 사용자의 접속을 해제합니다.\n",comm_sock[i]);
								strcpy(buf,"quit");
								if(send(comm_sock[i],buf,sizeof(buf),0) == -1){
									perror("send");
									exit(1);
								}
								close(comm_sock[i]);
								delete(comm_sock,waiting_room_n,i);
								waiting_room_n--;
								break;
							case 4://pong 게임 시작
								printf("%d 사용자가 ponggam을 시작합니다.\n",comm_sock[i]);
								strcpy(buf,PONGGAME);
								if(send(comm_sock[i],buf,sizeof(buf),0) == -1){
									perror("send");
									exit(1);
								}
								//close(comm_sock[i]);
								//delete(comm_sock,waiting_room_n,i);
								//waiting_room_n--;
								break;


							default:
								strcpy(buf,"메뉴번호를 다시 입력헤주세요\n");
								if(send(comm_sock[i],buf,sizeof(buf),0) == -1){
									perror("send");
									exit(1);
								}
								break;
						}
						ret--;
					}
				}
				else;
				i++;
			}
		}
	}
	return 0;
}

//채팅방 코드
void *t_function(void *data)
{	
	struct timeval t;
	t.tv_sec = 0;
	t.tv_usec = 500;
	fd_set readfds;
	char buf[BUFLEN];
	int ret;
	
	while(TRUE){
		struct ChatInfo *chatinfo=(struct ChatInfo*) data;
		
		//1. 새로운 채팅방 참가자가 있는지 확인
		int i = 0;
		while(chatinfo->new_users.user_num>0){
			chatinfo->room.user[chatinfo->room.user_num]=chatinfo->new_users.user[i];
			//새로운 사용자가 들어왔다는 출력문
			printf("[ch.%d]새로운 참가자 : %d\n",chatinfo->room.room_id,chatinfo->room.user[chatinfo->room.user_num]);
			delete(chatinfo->new_users.user,chatinfo->new_users.user_num,i);
			chatinfo->room.user_num++;
			chatinfo->new_users.user_num--;
			i++;	
		}
	
	
		//2. 채팅방에 참여한 사용자 중에서 메시지를 전송한 사람이 있는지 확인
		FD_ZERO(&readfds);for( int i = 0;i<chatinfo->room.user_num;i++) FD_SET(chatinfo->room.user[i],&readfds);
		ret = select(maxArr(chatinfo->room.user,chatinfo->room.user_num)+1,&readfds,NULL,NULL,&t);
		if(ret>0){
			int l = 0;
			while(ret>0){
				if(FD_ISSET(chatinfo->room.user[l],&readfds)){
					memset(buf,'\0',BUFLEN); // 버퍼 초기화

					//메세지를 받았다면
					if(recv(chatinfo->room.user[l],buf,BUFLEN,0) > 0) {
						printf("[ch.%d] 사용자 %d 의 메세지 : %s\n",chatinfo->room.room_id,chatinfo->room.user[l],buf); 
						
						//메세지가quit 라면
						if(strncmp(buf,QUITMSG,strlen(QUITMSG)) == 0){
							printf("[ch.%d] 사용자 %d를 채팅방에서 제거합니다.\n",chatinfo->room.room_id,chatinfo->room.user[l]);
							chatinfo->returned_users.user[chatinfo->returned_users.user_num]=chatinfo->room.user[l];
							chatinfo->returned_users.user_num++;
							delete(chatinfo->room.user,chatinfo->room.user_num,l);//채팅방에서 사용자 삭제
							chatinfo->room.user_num--;// 채팅방 인원수 감소
						}

						//자기자신 빼고 모두에게 메세지 전송
						char user_id[3];
						char send_buf[BUFLEN];
						
						sprintf(user_id,"%d",chatinfo->room.user[l]);
						memset(send_buf,'\0',BUFLEN);
						strcat(send_buf,"[");
						strcat(send_buf,user_id);
						strcat(send_buf,"] ");
						strcat(send_buf,buf);
						for(int i =0;i<chatinfo->room.user_num;i++){
							if(i == l) continue;
							if(send(chatinfo->room.user[i],send_buf,sizeof(buf),0) == -1){
									perror("send");
									exit(1);
							}
					
						}

					}
					ret--;
				}else;
				l++;
			}
						
		
		}
	}

	printf("");
}

//배열에서 최댓값 찾는 함수
int maxArr(int *n,int size){
	int max = n[0];
	for(int i = 0;i<size;i++){
		if(n[i]>max)max = n[i];
	}
	return max;
}

//배열원소 삭제 함수 
void delete(int *arr ,int size, int index){
	for(int i = index; i<size-1; i++){
		arr[i] = arr[i+1];
	}
}
//시크널 함수
void handler(int sig){
	//close(access_sock);
	//for(int i = 0; i<3;i++){

		//pthread_join(p_thread);
	//}

	exit(EXIT_SUCCESS);
}
