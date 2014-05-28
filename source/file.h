#ifndef _FILE_HEADER_
#define _FILE_HEADER_

	#include "INODE.h"

	typedef struct _file{
		INODE * inodeP; //文件的INODE对象
		int offset;//文件指针的当前偏移量
	}FILE_FS;

	char getc_FS(FILE_FS * fp);
	void * getContent_FS(FILE_FS * fp);

#endif
