#include "info.h"

//퐁게임 코드
void *t_ponggame(void *data)//*t_function
{	
	struct timeval t;
	t.tv_sec = 0;
	t.tv_usec = 500;
	fd_set readfds;
	char buf[BUFLEN];
	int ret;

	unsigned int time_num = 0; //시간 변수
       	clock_t start; //시작 시간	

	int pgs_num = 1 ; //pong_game_start을 한번만 보내기 위한 변수 
	int pong_game_start = 0;
	
	int ex_num = 0;
	while(TRUE){
		struct ChatInfo *chatinfo=(struct ChatInfo*) data;

			
		//1. 새로운 채팅방 참가자가 있는지 확인
		int i = 0;
		while(chatinfo->new_users.user_num>0){
			chatinfo->room.user[chatinfo->room.user_num]=chatinfo->new_users.user[i];
			//새로운 사용자가 들어왔다는 출력문
			printf("[ch.%d]새로운 참가자 : %d\n",chatinfo->room.room_id,chatinfo->room.user[chatinfo->room.user_num]);
			_delete(chatinfo->new_users.user,chatinfo->new_users.user_num,i);
			chatinfo->room.user_num++;
			chatinfo->new_users.user_num--;
			i++;	
		}
		//두명 되면 게임 시작 
		if(chatinfo->room.user_num == 2 && pgs_num)
		{
			memset(buf,'\0',BUFLEN);
			strcat(buf,"pong_game_start");
			for(int i =0;i<chatinfo->room.user_num;i++)
			{
				if(send(chatinfo->room.user[i],buf,sizeof(buf),0) == -1)
				{
					perror("send");
					exit(1);
				}
					
			}
			pgs_num = 0;
			pong_game_start =1;

		}
		else if(pong_game_start)
		{
			start = clock();
			time_num = (int)(((float)start/CLOCKS_PER_SEC)*2);
			if(ex_num != time_num)
			{
				//printf("num : %d\n",qqq);
				memset(buf,'\0',BUFLEN);
				strcat(buf,"ball_move");
				for(int i =0;i<chatinfo->room.user_num;i++)
				{
					
					if(send(chatinfo->room.user[i],buf,sizeof(buf),0) == -1)
					{
						perror("time_send");
						exit(1);
					}
					
						
				}
				ex_num = time_num;

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
								_delete(chatinfo->room.user,chatinfo->room.user_num,l);//채팅방에서 사용자 삭제
								chatinfo->room.user_num--;// 채팅방 인원수 감소
							}

							//자기자신 빼고 모두에게 메세지 전송--> 주석 처리 함 
							char user_id[3];
							char send_buf[BUFLEN];
							
							sprintf(user_id,"%d",chatinfo->room.user[l]);
							memset(send_buf,'\0',BUFLEN);
							if(PONGGAME == FALSE){
								strcat(send_buf,"[");
								strcat(send_buf,user_id);
								strcat(send_buf,"] ");
							}
							strcat(send_buf,buf);
							for(int i =0;i<chatinfo->room.user_num;i++){
								//if(i == l) continue; 자기 자신이라면 패스 
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
	}

	//printf("");
}
