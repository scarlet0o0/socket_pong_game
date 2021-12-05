#include "info.h"

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




//마피아 코드
void *t_mafiagame(void *data)
{	
	struct timeval t;
	t.tv_sec = 0;
	t.tv_usec = 500;
	fd_set readfds;
	char buf[BUFLEN];
	int ret;
	
	unsigned int time_num = 0; //시간 변수
	int ex_num = 0;	
	clock_t start; //시작 시간

	int mafia_num_set = FALSE;
	int mafia_start = FALSE;
	
	int mafia = 0;
	int police = 0;
	int pflag = 0; //경찰의 죽음 혹은 생존 
	while(TRUE){
		struct ChatInfo *chatinfo=(struct ChatInfo*) data;
		
		//1. 새로운 채팅방 참가자가 있는지 확인
		int i = 0;
		while(chatinfo->new_users.user_num>0){
			chatinfo->room.user[chatinfo->room.user_num]=chatinfo->new_users.user[i];
			//새로운 사용자가 들어왔다는 출력문
			printf("[ch.%d]마피아 게임 새로운 참가자 : %d\n",chatinfo->room.room_id,chatinfo->room.user[chatinfo->room.user_num]);
			_delete(chatinfo->new_users.user,chatinfo->new_users.user_num,i);
			chatinfo->room.user_num++;
			chatinfo->new_users.user_num--;
			i++;	
		}
		//방인원수 5명 되면 시작 
		if(chatinfo->room.user_num == 5 && !mafia_num_set)
		{
			memset(buf,'\0',BUFLEN);
			strcat(buf,"mafia_game_start");
			/*
			for(int i =0;i<chatinfo->room.user_num;i++)
			{
				if(send(chatinfo->room.user[i],buf,sizeof(buf),0) == -1)
				{
					perror("send");
					exit(1);
				}
					
			}
			*/
			mafia_num_set = TRUE;
			mafia_start = TRUE;

		}

		if(mafia_start){
			printf("마피아 게임을 시작합니다.\n");		

			srand(time(NULL));
			mafia = rand() % 5; //마피아 정하기
			police = (mafia + 1) % 5;// 경찰 정하기
			
			//참여자들에게 게임 시작 메시지 보내기
			for(int k = 0; k < chatinfo->room.user_num; k++)
			{
				send(chatinfo->room.user[k],start_command2,sizeof(start_command2),0); 
			}
			
			//역할 이야기 해주기
			for (int k = 0; k < chatinfo->room.user_num; k++){
				send(chatinfo->room.user[k], morning, strlen(morning), 0);
				if (k == mafia)//마피아에게 마피아라고 메세지 보내기
					send(chatinfo->room.user[k], mfmsg, strlen(mfmsg), 0);
				if(k == police && pflag == 0)//경찰에게 경찰이라고 메세지 보내기
					send(chatinfo->room.user[k],plcmsg,strlen(plcmsg),0);
				if(k != mafia && k != police)
					send(chatinfo->room.user[k], ctzmsg, strlen(ctzmsg), 0);
				send(chatinfo->room.user[k],chating, strlen(chating), 0);//채팅 시작 메세지 전송i

			}

			int end_count =0;

			//낮 투표 시간동안 토론
			while(TRUE)
			{
				//타이버 부분
				start = clock();
				time_num = (int)(((float)start/CLOCKS_PER_SEC)*2);
				if(ex_num != time_num)
				{
					if(time_num % 5)
						end_count++;
					printf("time_num : %d\n",time_num);
					ex_num = time_num;
				}
				if(end_count == 10)
					break;

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
								//자기자신 빼고 모두에게 메세지 전송--> 주석 처리 함 
								char user_id[3];
								char send_buf[BUFLEN];
								
								sprintf(user_id,"%d",chatinfo->room.user[l]);
								memset(send_buf,'\0',BUFLEN);
								strcat(send_buf,"[");
								strcat(send_buf,user_id);
								strcat(send_buf,"] ");
								strcat(send_buf,buf);
								for(int i =0;i<chatinfo->room.user_num;i++){
									if(i == l) continue; //자기 자신이라면 패스 
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
	}

	printf("");
}
