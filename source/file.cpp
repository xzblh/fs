#include <stdio.h>

#include "superBlock.h"
#include "file.h"

extern FILE * dataFp;
extern SUPER_BLOCK * superBlockPointer;

int writeContent(INODE * inodeP, void * mem, int length, int offset)
{
	if(length + offset >= getFileSizeLimit(superBlockPointer)){
		//printf("超出单文件大小限！\r\n");
		return -1;
	}
	int l = ftell(dataFp);
	fseek(dataFp, inodeP->blockNumber * superBlockPointer->blockSize, SEEK_SET);
	fwrite(mem, length, 1, dataFp);
	fseek(dataFp, l, SEEK_SET);
	return 0;
}

int createFile(INODE * inodeP, char * fileName)
{
	if(!isDir(inodeP)){
		return -1;
	}
	INODE * tmp = createINODE();
}


char getc_FS(FILE_FS * fp)
{
	INODE * tmp_inode_pointer;
	tmp_inode_pointer = fp->inodeP;
	return ' ';
}

void * getContent_FS(FILE_FS * fp)
{
	return NULL;
}
