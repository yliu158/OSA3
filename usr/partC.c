#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

/* allocate memory for character buffers HERE before you use them */
#define N 10
#define MAX_LENGTH 100
//#define O_RDONL 

struct timeval gtodTimes[N];
char *procClockTimes[N];
#define O_RDONL 
#define MEMORY_SIZE 256

int main(){
	int i;	
	
	struct timeval gtodTimes[N];
	char *procClockTimes[N];
	for (i = 0; i < N; i++) {
		procClockTimes[i] =(char *) malloc(MEMORY_SIZE);
	}
	/* allocate memory for character buffers HERE before you use them */
	int fd = open("/dev/mytime", O_RDONLY);
	/* check for errors HERE */
	if (fd < 0) {
		printf("Fail to open file.\n");
	}
	for( i=0; i < N; i++)
	{
		gettimeofday(&gtodTimes[i], 0);
		int bytes_read = read(fd, procClockTimes[i], MAX_LENGTH);
		/* check for errors HERE */
		if (bytes_read == 0) {
			printf("Successfully read.\n");
		} else {
			printf("Fail to read.\n");
		}
	}
	
	close(fd);

	for(i=0; i < N; i++) {
		printf("Current Kernel Time: %ld\n", gtodTimes[i].tv_sec);
		printf("getnstimeofday: %s\n", procClockTimes[i]);
		/* fix the output format appropriately in the above line */
	}
	for(i = 0; i < N; i++) {
		free(procClockTimes[i]);
	}
}
