#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "ioctl_test.h"

int main() {
	struct mystruct message;
	printf("Enter the message\n");
	scanf("%[^\n]s",message.msg);
	printf("Enter the delay\n");
	scanf("%d",&message.delay);
	int dev = open("/dev/ioctl", O_WRONLY);
	if(dev == -1) {
		printf("Opening was not possible!\n");
		return -1;
	}

	ioctl(dev, WR_VALUE, &message);
	printf("Program1 written to driver\n");
	ioctl(dev, RD_VALUE, &message);
	printf("Program1 read from driver\n");
	printf("Driver's message : %s\n",message.msg );
	printf("Operation done\n");
	close(dev);
	return 0;
}
