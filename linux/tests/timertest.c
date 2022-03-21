/*                                 TIMERTEST.C                              */
/*  This test file tests the functions in timer.c                           */
#include <stdio.h>
#include <time.h>
#include <unistd.h>

typedef unsigned int bit_32;
typedef unsigned short bit_16;
typedef char *char_ptr;

char time_array[16];

#define Bit_16(exp)	((bit_16) (exp))

bit_32 get_time(void);
char_ptr elapsed_time(bit_32 start_time, bit_32 stop_time);

int main(void)
{
	bit_32 btime = 0, etime = 0, stime=0;

	for(int i = 1; i <= 5; i++)
	{
		btime = get_time();
		if(!stime)
			stime = btime;
		sleep(i);
		etime = get_time();

		printf("i is %d, elapsed time in seconds is %s\n", i, elapsed_time(btime, etime));
	}

	printf("Total elapsed run time in seconds is %s\n", elapsed_time(stime, etime));
	return 0;
}

#include "../timer.c"
