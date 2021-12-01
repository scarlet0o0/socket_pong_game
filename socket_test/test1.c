#include <ncurses.h>
#include <string.h>
#include <locale.h> 
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

int main()
{
	setlocale(LC_ALL,"ko_KR.utf8");//한글이 출력 가능하게 

	//WINDOW* 특정 윈도우를 가르키는 포인터 데이터 타입
	WINDOW *win_input;
	WINDOW *win_output;
	WINDOW *win_output_panel;
	WINDOW *win_input_panel;

	int i;
	char buf[100];
	char c;
	const char *exit = "exit";

	initscr();
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
	mvwprintw(win_output, 1, 10, "window내에 글자쓰기 테스트!!\n");
	wrefresh(win_output);

	//noecho();

	/*
	pthread_t tID1, tID2;
	int ret;

	Data data;
	data.w_input = win_input;
	data.w_output = win_output;
	//data.s = sock1;
	data.pong_game_on = FALSE;
	 
	//wprintw(win_output, "%s\n", "wqe");

	pthread_create(&tID1, NULL, start_f1,(void *) &data);
	pthread_create(&tID2, NULL, start_f2,(void *) &data);
	
	pthread_join(tID1,NULL);
	pthread_join(tID2,NULL);

	pthread_detach(tID1);
	pthread_detach(tID2);
	*/
	 
	
	while(1)
	{
		
		//mvwgetstr(win_input, 1, 1, buf);//문자열을 입력받으면 buf에 저장 
		snprintf(buf,sizeof(buf),"%c",getch());
		//c = getch();
		if(NULL != strstr(buf, "exit"))//exit인 경우 나감 
		{
			break;
		}
		else
		{
			wprintw(win_output, "%c\n", buf[0]);//출력 
			//winsch(win_output,buf[0]);
		}
		wclear(win_input);
		wrefresh(win_output);
		wrefresh(win_input);
	}
	

	endwin();
}
/*
void *start_f1(void *data)
{
	Data *a = (Data *)data;
	char buf[BUFLEN];
	char c;
	wprintw(a->w_output, "%c\n", "wqe");


	while(TRUE)
	{
		mvwgetstr(a->w_input, 1, 1, buf);//문자열을 입력받으면 buf에 저장
		wprintw(a->w_output, "%s\n", buf);
		wclear(a->w_input);
		wrefresh(a->w_output);
		wrefresh(a->w_input);
	}

	pthread_exit(0); //return 구문 대신, 리턴값을 인자로 전달 		
}

void *start_f2(void *data)
{
	Data *a = (Data *)data;
	char buf[BUFLEN];

	while(TRUE)
	{
		memset(buf,'\0',BUFLEN);
		if(TRUE)
		{
			//wprintw(win_output, "%c\n", buf[0]);
		}
		
		if(a->pong_game_on)
		{
			object scr;
			int i = 0,cont = 0;
			int end = 0;
			char k;

			noecho();
			curs_set(0);
			getmaxyx(a->w_output,scr.y,scr.x);
			object b1={scr.x-2,scr.y/2,0,false,false};//플레이어1 막대기 선언
			object b2={1,scr.y/2,0,false,false};//플레이어2 막대기 선언
			object b={scr.x/2,scr.y/2,0,false,false};// 공 구조체 선언

  			for (nodelay(stdscr,1); !end; usleep(4000)) {
   			if (++cont%16==0)//볼 속도 조절 
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

			memset(buf,'\0',BUFLEN);
			if(recv(a->s,buf,BUFLEN,0) > 0)
			{
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
				//pthread_mutex_lock(&mutex);
				wprintw(a->w_output, "%s\n", buf);//출력
				//pthread_mutex_unlock(&mutex);
				wrefresh(a->w_output);
			}	
			k = buf[4];
    			switch (k) // 키 입력 받기
    			{
      				case 'u': b1.y++; break;// 방향키(위) 입력시 플레이어1 막대기 위로 
      				case 'j':   b1.y--; break;// 방향키(아래) 입력시 플레이어1 막대기 아래로
      				case 'q':      b2.y--; break;// q입력시 플레이어2 막대기 위로
      				case 'a':      b2.y++; break;// a입력시 플레이어2 막대기 아래로 
      				case 'p':      getchar(); break;
      				case 0x1B:    endwin(); end++; break;
    			}
    			erase();
    			mvprintw(2,scr.x/2-2,"%i | %i",b1.c,b2.c);//점수판 출력
    			mvvline(0,scr.x/2,ACS_VLINE,scr.y);
    			attron(COLOR_PAIR(1));
	
    			mvprintw(b.y,b.x,"o");//볼 출력
			//막대기 출력
    			for(i=-1;i<2;i++)
			{
      				mvprintw(b1.y+i,b1.x,"|");
      				mvprintw(b2.y+i,b2.x,"|");}
    				attroff(COLOR_PAIR(1));
  			}
		
				
		}
		

		usleep(50);
		
	}

	pthread_exit(0); //return 구문 대신, 리턴값을 인자로 전달 
		
}
*/
