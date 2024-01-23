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
    printf("Enter the message\n");
	scanf("%[^\n]s",message);
	int ret =ioctl(dev, WR_VALUE, message);
	if (ret==-1)
	{
		printf("Buffer full read first\n");
		return -1;
	}
	printf("Program written to driver\n");
	close(dev);
	return 0;
}
