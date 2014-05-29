#include <time.h>

#ifndef _INODE_HEADER_
#define _INODE_HEADER_

	/*
	* 按照下面的结构计算，每个inode大小为4*6 = 24字节
	*/
	typedef struct _inode{
		time_t aTime; //通通转成时间戳  数字存储
		time_t cTime;
		time_t mTime;
		unsigned int authority;
		int inodeNumber; //INODE节点的编号
		int blockNumber; // 扇区块编号，一重间接方式使用 即单文件大小限制：512*512/4 = 64K
		int length; // 文件长度
	}INODE;

#endif
