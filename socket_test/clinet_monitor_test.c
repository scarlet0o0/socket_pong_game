#include "info.h"

typedef struct Data
{
	WINDOW *w_input;
	WINDOW *w_output;
	int s;
	int pong_game_on;
}Data;

typedef struct
{
	short int x, y, c; // x,y는 좌표값 c는 점수 
	int movhor, movver;
} object;

void *start_f1(void *data);
void *start_f2(void *data);

pthread_mutex_t mutex;

int main(int argc, char*argv[]){

	pthread_mutex_init(&mutex,NULL);
	
	//ncurses =====================================================
	setlocale(LC_ALL,"ko_KR.utf8");

	WINDOW *win_input;
	WINDOW *win_output;
	WINDOW *win_output_panel;
	WINDOW *win_input_panel;

	int i;
	//char buf[BUFLEN];
	
	initscr();
	resizeterm(40,130);
	refresh();

	//newwin(행크기,열크기,시작y좌표,시작x좌표);
	
	win_output = newwin(LINES - 7, COLS - 2, 1, 1);//윈도우 창 만들기 
	scrollok(win_output, TRUE);
	wrefresh(win_output);//특정 윈도우를 새로고치는 함수 
	

	win_output_panel = newwin(LINES - 5, COLS, 0, 0);//윈도우 창 만들기 
	box(win_output_panel, 0, 0);//윈도우 박스 만들기 
	wrefresh(win_output_panel);

	win_input_panel = newwin(5, COLS, LINES - 5 , 0);//윈도우 창 만들기 
	box(win_input_panel, 0, 0);
	wrefresh(win_input_panel);

	win_input = newwin(3, COLS - 2, LINES - 4 , 1);
	scrollok(win_input, TRUE);
	wrefresh(win_output);

	start_color();
	init_pair(1,COLOR_BLUE,COLOR_BLACK);

	//==================================================================

		
	//서버와 연결위한 설정============================
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
	//=================================================

	
	//서버로 연결신호를 보낸다
	if(connect(sock1,(struct sockaddr *)&server1,sizeof(server1))){
		perror("server");exit(1);
	}

	wprintw(win_output,"%d, %d\n",LINES,COLS);
	wprintw(win_output,"%s\n","서버와 연결됨");
	wrefresh(win_output);

	int flags1 = fcntl(sock1,F_GETFL,0);
	fcntl(sock1,F_SETFL,flags1|O_NONBLOCK);

	//클라이언트 인풋과 메시지를 주고 받는다.
	
	pthread_t tID1, tID2;
	int ret;

	Data data;
	data.w_input = win_input;
	data.w_output = win_output;
	data.s = sock1;
	data.pong_game_on = FALSE;

	pthread_create(&tID1, NULL, start_f1,(void *) &data);
	pthread_create(&tID2, NULL, start_f2,(void *) &data);
	
	pthread_join(tID1,NULL);
	pthread_join(tID2,NULL);

	pthread_detach(tID1);
	pthread_detach(tID2);
	
	pthread_mutex_destroy(&mutex);

	close(sock1);
	
	endwin();
	return 0;
}


void *start_f1(void *data)
{
	Data *a = (Data *)data;
	char buf[BUFLEN];
	char c;

	while(TRUE)
	{
		while(!a->pong_game_on)
		{
			memset(buf,'\0',BUFLEN);

			//pthread_mutex_lock(&mutex);
			mvwgetstr(a->w_input, 1, 1, buf);//문자열을 입력받으면 buf에 저장 
			//pthread_mutex_unlock(&mutex);
		
				//wprintw(a->w_output, "%s\n", buf);//출력
			pthread_mutex_lock(&mutex);
			if(send(a->s,buf,sizeof(buf),0) == -1)
			{
				perror("send");break;
			}
			werase(a->w_input);
			wrefresh(a->w_input);
			pthread_mutex_unlock(&mutex);
		}

		while(a->pong_game_on)
		{
			noecho();
			while(TRUE){
			memset(buf,'\0',BUFLEN);
			snprintf(buf,sizeof(buf),"%c",getch());
			pthread_mutex_lock(&mutex);
			//wprintw(a->w_output, "%s\n", buf);//출력
			if(send(a->s,buf,sizeof(buf),0) == -1)
			{
				perror("send");break;
			}
			werase(a->w_input);
			//wrefresh(a->w_output);
			wrefresh(a->w_input);
			pthread_mutex_unlock(&mutex);
			usleep(50);
			}

		}
		usleep(50);

	}
	pthread_exit(0); //return 구문 대신, 리턴값을 인자로 전달 		
}

void *start_f2(void *data)
{
	Data *a = (Data *)data;
	char buf[BUFLEN];

	while(TRUE)
	{
		while(!a->pong_game_on)
		{
			memset(buf,'\0',BUFLEN);
			pthread_mutex_lock(&mutex);
			if(recv(a->s,buf,BUFLEN,0) > 0 && a->pong_game_on == FALSE)
			{
				//pthread_mutex_unlock(&mutex);
				if(strncmp(buf,PONGGAME,strlen(PONGGAME))==0)
				{
					//printf("[INFO]소켓을 닫고 프로그램을 종료합니다\n");
					a->pong_game_on = TRUE;
				}
				if(strncmp(buf,QUITMSG,strlen(QUITMSG))==0)
				{
					printf("[INFO]소켓을 닫고 프로그램을 종료합니다\n");
					break;
				}
				wprintw(a->w_output, "%s\n", buf);//출력
				wrefresh(a->w_output);
			}
			pthread_mutex_unlock(&mutex);
			usleep(4000);
		}

		while(a->pong_game_on)
		{								
			werase(a->w_output); 
			
			object scr;
			int i = 0,cont = 0;
			int pong_game_start = 0;
			char k;

			noecho();
			curs_set(0);
			getmaxyx(a->w_output,scr.y,scr.x);
			object b1={scr.x-2,scr.y/2,0,false,false};//플레이어1 막대기 선언
			object b2={1,scr.y/2,0,false,false};//플레이어2 막대기 선언
			object b={scr.x/2,scr.y/2,0,false,false};// 공 구조체 선언

			//타이틀를 보여준다.
  			mvwprintw(a->w_output,12,0,"\t\t\t\t           oooooooooo                                        \n"
               	     		     		 "\t\t\t\t           888    888  ooooooo    ooooooo    oooooooo8       \n"
                     	   	     		 "\t\t\t\t           888oooo88 888     888 888   888  888    88o       \n"
                     		     		 "\t\t\t\t           888       888     888 888   888   888oo888o       \n"
                     		     		 "\t\t\t\t          o888o        88ooo88  o888   888o         888      \n"
                     		     		 "\t\t\t\t                                             888ooo888     \n\n"
                     		     		 "\t\t\t\t  \tPlayer 1 your controls are 'a' and 'q'                \n"
                     		     		 "\t\t\t\t  \tPlayer 2 your controls are the arrows of the keyboard \n"
                     		     		 "\t\t\t\t \tPush ANY key to start, 'p' for pause and ESC to quit" ); 
			wrefresh(a->w_output);

			if(recv(a->s,buf,BUFLEN,0) > 0)
			{
					//pthread_mutex_unlock(&mutex);
				if(strncmp(buf,"pong_game_start",strlen("pong_game_start"))==0)
				{
					pong_game_start = 1;
				}
			}


  			for (; pong_game_start; usleep(4000)) 
			{
				




				memset(buf,'\0',BUFLEN);
				pthread_mutex_lock(&mutex);
				
				if(recv(a->s,buf,BUFLEN,0) > 0)
				{
					if (strncmp(buf,"ball_move",strlen("ball_move"))==0)//볼 속도 조절 
					{

						if ((b.y==scr.y-1)||(b.y==1))
						b.movver=!b.movver;
						if ((b.x>=scr.x-2)||(b.x<=2))
						{
							b.movhor=!b.movhor;
							if ((b.y==b1.y-1)||(b.y==b2.y-1)) 
							{
								b.movver=false;
							} 
							else if ((b.y==b1.y+1)||(b.y==b2.y+1)) 
							{
								b.movver=true;
							} 
							else if ((b.y != b1.y) && (b.y != b2.y)) 
							{
								(b.x>=scr.x-2) ? b1.c++: b2.c++;
								b.x=scr.x/2;
								b.y=scr.y/2;
							}
						}
						b.x=b.movhor ? b.x+1 : b.x-1;
						b.y=b.movver ? b.y+1 : b.y-1;
		
						if (b1.y<=1)
							b1.y=scr.y-2;
						if (b1.y>=scr.y-1)
							b1.y=2;
						if (b2.y<=1)
							b2.y=scr.y-2;
						if (b2.y>=scr.y-1)
							b2.y=2;
					}
					k = buf[0];
					switch (k) // 키 입력 받기
					{
						case 'j': b1.y++; break;// 방향키(위) 입력시 플레이어1 막대기 위로 
						case 'u':   b1.y--; break;// 방향키(아래) 입력시 플레이어1 막대기 아래로
						case 'q':      b2.y--; break;// q입력시 플레이어2 막대기 위로
						case 'a':      b2.y++; break;// a입력시 플레이어2 막대기 아래로 
						case 'p':      getchar(); break;
						case 0x1B:    endwin(); break;
					}

				}
				
				pthread_mutex_unlock(&mutex);
				
    				werase(a->w_output); // 전체 창 지우기
				
    				mvwprintw(a->w_output,2,scr.x/2-2,"%i | %i",b1.c,b2.c);//점수판 출력
    				mvwvline(a->w_output,0,scr.x/2,ACS_VLINE,scr.y);
    				attron(COLOR_PAIR(1));
	
    				mvwprintw(a->w_output,b.y,b.x,"o");//볼 출력
				//막대기 출력
    				for(i=-1;i<2;i++)
				{
      					mvwprintw(a->w_output,b1.y+i,b1.x,"|");
      					mvwprintw(a->w_output,b2.y+i,b2.x,"|");
				}
    				attroff(COLOR_PAIR(1));
				
				wrefresh(a->w_output);
  			}
		

		}
		//sleep(50);	
	}
	pthread_exit(0); //return 구문 대신, 리턴값을 인자로 전달 		
}

