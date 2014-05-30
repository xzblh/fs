#ifndef _FILE_HEADER_
#define _FILE_HEADER_

	#include "INODE.h"

	typedef struct _file{
		INODE * inodeP; //文件的INODE对象
		int offset;//文件指针的当前偏移量
		void * mem; //间接block的内容
	}FILE_FS;

	char getc_FS(FILE_FS * fp);
	void * getContent_FS(FILE_FS * fp);

#endif


int writeContent(INODE * inodeP, void * mem, int length, int offset);
				//  确定文件INODE编号   要写的内存区  要写的内存长度    将要写的文件偏移量
int createFile(INODE * inodeP, char * fileName);
int createDir(INODE * inodeP, char * dirName);