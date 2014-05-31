#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "superBlock.h"
#include "file.h"
#include "tool.h"

extern char * currentPwd;
extern User * currentUser;
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
	if(strlen(fileName) > 27){
		//文件名长度限制 必须小于等于27，最长不能超过27
		return -4;
	}
	if(!isDir(inodeP)){
		return -1;
	}
	if(!hasCreateFileAuthority(inodeP, currentUser)){
		return -2;
	}
	INODE * tmpInodePointer = NULL;
	char str[32];
	memset(str, 0, 32);
	strcpy(str, fileName);
	BLOCK * blockP;
	if(inodeP->length % superBlockPointer->blockSize == 0){
		//之前分配的block刚好用完。
		if(inodeP->length + 32 >= getFileSizeLimit(superBlockPointer)){
			//单文件大小限制
			return -3;
		}
		else{
			//获得为原来文件夹新增的扇区
			blockP = createBlock();
			//在原来文件夹的间接扇区上增加新增的扇区编号
			((int*)inodeP->mem)[inodeP->length/4] = blockP->blockNumber;
		}
	}
	else{
		blockP = getBlock(inodeP->blockNumber);
	}
	//获得新文件的INODE
	tmpInodePointer = createINODE(_755_AUTHORITY_FILE_);

	//把文件名和文件的INODE编号组合在32个字节里面。
	*(unsigned int*)(str+28) = blockP->blockNumber;
	//分配一块内存，方便写入
	void * mem = Malloc(superBlockPointer->blockSize);
	memset(mem, 0, superBlockPointer->blockSize);
	strcpy((char*)mem, str);

	//在新扇区上添加新增的文件数据。 及文件名和文件的INODE编号
	writeBlock(blockP, mem);
	free(blockP);
	inodeP->length += 32; //单文件名27个字符限制  27个字符+1个结束标志+4个Inode指示，总共32位。
	writeINODE(inodeP);
	writeINODE(tmpInodePointer);
	return 0;
}

int createDir(INODE * inodeP, char * dirName)
{
	if(!hasCreateFileAuthority(inodeP, currentUser)){
		return -2;
	}
	INODE * tmp = createINODE(_755_AUTHORITY_DIR_);
	return 0;
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

FILE_FS * openFile(char * fileName)
{
	if(NULL == fileName){
		return NULL;
	}
	char path[512];
	FILE_FS * fileFsP = (FILE_FS *)Malloc(sizeof(FILE_FS));
	fileFsP->mem = Malloc(superBlockPointer->blockSize);
	if(fileName[0] == '/'){
		fileFsP->inodeP = getInode(fileName);
	}
	else{
		strcpy(path, currentPwd);
		strcat(path, fileName);
		fileFsP->inodeP = getInode(path);
	}
}