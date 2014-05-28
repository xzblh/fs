#include <time.h>

#ifndef _INODE_HEADER_
#define _INODE_HEADER_

typedef struct inode{
	time_t aTime;
	time_t cTime;
	time_t mTime;
	int authority;
};