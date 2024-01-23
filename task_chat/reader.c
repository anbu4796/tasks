#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "ioctl_test.h"

int main() {
    char message[64];
	int dev = open("/dev/ioctl", O_WRONLY);
	if(dev == -1) {
		printf("Opening was not possible!\n");
		return -1;
	}

	
	int ret=ioctl(dev, RD_VALUE, &message);
	if (ret==-1)
	{
		printf("Buffer empty write first\n");
		return -1;
	}
	
	printf("Program2 read from driver\n");
	printf("Driver's message : %s\n",message );
	close(dev);
	return 0;
}
