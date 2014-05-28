#include <time.h>

#ifndef _INODE_HEADER_
#define _INODE_HEADER_

	typedef struct _inode{
		time_t aTime;
		time_t cTime;
		time_t mTime;
		int authority;
		int inodeNumber; //INODE节点的编号
		int blockNumber; // 扇区块编号，一重间接方式使用
	}INODE;

#endif
