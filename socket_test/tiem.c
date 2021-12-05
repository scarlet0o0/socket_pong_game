#include<time.h>
#include<stdio.h>

int main()
{
	unsigned int num =0;
	clock_t start, end;

	start = clock();
	int num2 =0;
	while(1)
	{
		start = clock();
		num =(int)(((float)start/CLOCKS_PER_SEC*2));
		if(num2 !=num)
		{
			if(num == 10)
				break;
			printf("num : %d\n",num);
			num2 = num;

		}
		
		
	}

	//printf("time --> %f\n",(float)(end - start)/CLOCKS_PER_SEC);

	return 0;
}
			
