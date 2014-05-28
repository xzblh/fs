#include <time.h>

#ifndef _SUPERBLOCK_HEADER_
#define _SUPERBLOCK_HEADER_
	typedef struct _superblock{
		int blockCount;
		int inodeCount;
		int blockFreeCount;
		int inodeFreeCount;
		int blockSize; //对于本测试程序来说，固定512字节
		int inodeSize; //对于本测试程序来说，固定512字节
		time_t mountTime; //挂载时间
	}SUPER_BLOCK;
#endif