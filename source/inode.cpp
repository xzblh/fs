#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"
#include "block.h"
#include "superBlock.h"
#include "INODE.h"
#include "tool.h"
#include "block.h"


extern FILE * dataFp;
extern SUPER_BLOCK * superBlockPointer;
extern User * currentUser;


/************************************************************************/
/* 获得一个未使用的INode，一个未使用的block，并初始化这个block为全0     */
/************************************************************************/
INODE * createINODE(unsigned int authority)
{
	int inodePos = findZero(superBlockPointer->iBitMap, getInodeBitMapByteCount(superBlockPointer));
	INODE * inodeP = (INODE*)Malloc(sizeof(INODE));
	inodeP->mem = Malloc(superBlockPointer->blockSize);
	inodeP->inodeNumber = inodePos;
	inodeP->GID = currentUser->GID;
	inodeP->UID = currentUser->UID;
	time(&inodeP->cTime); //当前时间
	memcpy(&inodeP->mTime, &inodeP->cTime, sizeof(time_t)); //保证三个时间一致
	memcpy(&inodeP->aTime, &inodeP->cTime, sizeof(time_t));
	//time(&inodeP->mTime); //修改时间
	//time(&inodeP->aTime); //访问时间
	inodeP->authority  = authority;

	BLOCK * blockP = createBlock();
	inodeP->blockNumber = blockP->blockNumber;
	freeBlock(blockP);
	inodeP->length = 0;
	return inodeP;
}

INODE * getINODE(int inodeNumber)
{
	INODE * inodeP = (INODE *)Malloc(sizeof(INODE));
	inodeP->mem = Malloc(superBlockPointer->blockSize);
	int time_tmp = 0;
	fseek(dataFp, getInodeAreaOffset(superBlockPointer) + inodeNumber * superBlockPointer->inodeSize, SEEK_SET);
	fread(&time_tmp, 4, 1, dataFp);
	inodeP->aTime = time_tmp;
	fread(&time_tmp, 4, 1, dataFp);
	inodeP->cTime = time_tmp;
	fread(&time_tmp, 4, 1, dataFp);
	inodeP->mTime = time_tmp;
	fread(&inodeP->GID, 4, 1, dataFp);
	fread(&inodeP->UID, 4, 1, dataFp);
	fread(&inodeP->authority, 4, 1, dataFp);
	fread(&inodeP->inodeNumber, 4, 1, dataFp);
	fread(&inodeP->blockNumber, 4, 1, dataFp);
	fread(&inodeP->length, 4, 1, dataFp);
	fseek(dataFp, getBlockOffset(inodeP->blockNumber), SEEK_SET);
	fread(inodeP->mem, superBlockPointer->blockSize, 1, dataFp);
	return inodeP;
}

void writeINODE(INODE * inodeP)
{
	//data.txt的区块偏移， 加上INODE节点区的偏移
	int offset = inodeP->inodeNumber * superBlockPointer->inodeSize + getInodeAreaOffset(superBlockPointer);
	if(fseek(dataFp, offset, SEEK_SET) == 0){
		int time_tmp = inodeP->aTime;
		Fwrite(&time_tmp, 4, 1, dataFp);
		//fwrite(&inodeP->aTime, 4, 1, dataFp);
		time_tmp = inodeP->cTime;
		Fwrite(&time_tmp, 4, 1, dataFp);
		//fwrite(&inodeP->cTime, 4, 1, dataFp);
		time_tmp = inodeP->mTime;
		Fwrite(&time_tmp, 4, 1, dataFp);
		//fwrite(&inodeP->mTime, 4, 1, dataFp);
		Fwrite(&inodeP->GID, 4, 1, dataFp);
		Fwrite(&inodeP->UID, 4, 1, dataFp);
		Fwrite(&inodeP->authority, 4, 1, dataFp);
		Fwrite(&inodeP->inodeNumber, 4, 1, dataFp);
		Fwrite(&inodeP->blockNumber, 4, 1, dataFp);
		Fwrite(&inodeP->length, 4, 1, dataFp);
		BLOCK block;
		block.blockNumber = inodeP->blockNumber;
		writeBlock(&block, inodeP->mem);
		//fwrite(inodeP, sizeof(INODE), 1, dataFp);
	}
	else{
		printf("文件指针偏移失败，请重新打开程序尝试，不成功则程序已崩溃……！\r\n");
		exit(-255);
	}
}

BOOL isDir(INODE * inodeP)
{
	if(inodeP->authority && _DIR_DEFINE_)
		return TRUE;
	else
		return FALSE;
}

void freeInode(INODE * inodeP)
{
	free(inodeP->mem);
	free(inodeP);
}

void inodeMemAddBlock(INODE * inodeP, int blockNumber)
{
	int * p = (int *)inodeP->mem;
	p[inodeP->length / superBlockPointer->blockSize] = blockNumber;
	//刷新磁盘
	writeINODE(inodeP);
}