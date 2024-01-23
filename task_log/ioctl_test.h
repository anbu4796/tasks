#ifndef IOCTL_TEST_H
#define IOCTL_TEST_H

struct mystruct{
	int delay;
	char msg[64];
};

#define WR_VALUE _IOW('a', 'a', int32_t *)
#define RD_VALUE _IOR('a', 'b', int32_t *)


#endif
