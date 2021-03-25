#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char * argv [])
{
	int fd;
	int on = 0;
	fd = open("/dev/led0",O_RDWR);
	if(fd < 0)
	{
		perror("open");
		exit(1);
	}

	while(1)
	{
		on = 1;
		printf("user = %d\n",on);
		write(fd,&on,4);
		sleep(3);

		on = 0;
		printf("user = %d\n",on);
		write(fd,&on,4);
		sleep(3);
		
		
	}

	close(fd);
	return 0;
}

