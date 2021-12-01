#include <ncurses.h>
#include <unistd.h>
typedef struct
{
	short int x, y, c; // x,y는 좌표값 c는 점수 
	bool movhor, movver;
} object;

int main() {
	object scr; 
	int i = 0,cont=0; 
	bool end=false;

	initscr();// 가장먼저 호출되어야하고 기본크기 윈도우 생성
	start_color();//색을 사용하기전에 반드시 선언
 	init_pair(1,COLOR_BLUE,COLOR_BLACK);//한쌍의 글자색과 배경색 atttribute를 지정
  	keypad(stdscr,true);//키보드의 특수 키의 입력을 가능하게 설정하는 함수 
  	noecho();//문자를 입력할때에 입력한 값을 화면에 출력하지 않는 함수 
  	curs_set(0);//화면에 보이는 커서를 설정 가능 인자가 0일시 커서 사라짐 
  	getmaxyx(stdscr,scr.y,scr.x);//현재 화면에 세로값->scr.y에 가로값->scr,x에 삽입

  	object b1={scr.x-2,scr.y/2,0,false,false};//플레이어1 막대기 선언
	object b2={1,scr.y/2,0,false,false};//플레이어2 막대기 선언
	object b={scr.x/2,scr.y/2,0,false,false};// 공 구조체 선언

		

	//타이틀를 보여준다.
  	mvprintw(4,0,"\t           oooooooooo                                        \n"
               	     "\t           888    888  ooooooo    ooooooo    oooooooo8       \n"
                     "\t           888oooo88 888     888 888   888  888    88o       \n"
                     "\t           888       888     888 888   888   888oo888o       \n"
                     "\t          o888o        88ooo88  o888o o888o 888     888      \n"
                     "\t                                             888ooo888     \n\n"
                     "\t Any questions please send me at vicente.bolea@gmail.com     \n"
                     "\t \t\t\tPlayer 1 your controls are 'a' and 'q'                \n"
                     "\t \t\t\tPlayer 2 your controls are the arrows of the keyboard \n"
                     "\t \t\t\tPush ANY key to start, 'p' for pause and ESC to quit" ); 

  	getch();//키보드 글자 입력시 게임 시작 
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

    	switch (getch()) // 키 입력 받기
    	{
      		case KEY_DOWN: b1.y++; break;// 방향키(위) 입력시 플레이어1 막대기 위로 
      		case KEY_UP:   b1.y--; break;// 방향키(아래) 입력시 플레이어1 막대기 아래로
      		case 'q':      b2.y--; break;// q입력시 플레이어2 막대기 위로
      		case 'a':      b2.y++; break;// a입력시 플레이어2 막대기 아래로 
      		case 'p':      getchar(); break;
      		case 0x1B:    endwin(); end++; break;
    	}
    	werase(stdscr);
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
