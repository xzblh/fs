#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
	int inodePos = getFreeInodeNumber(superBlockPointer);
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

INODE * copyINODE(INODE * inodeP)
{
	INODE * r = (INODE * )Malloc(sizeof(INODE));
	void * mem = Malloc(superBlockPointer->blockSize);
	r->mem = mem;
	memcpy(r, inodeP, sizeof(INODE));
	memcpy(mem, inodeP->mem, superBlockPointer->blockSize);
	return r;
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
		getchar();
		exit(-255);
	}
}

void writeINODEData(INODE * inodeP, char c) //往INODE数据区结尾写数据
{
	if((inodeP->length+1) % superBlockPointer->blockSize == 0){
		//需要新增扇区

		//获得为原来文件夹新增的扇区
		BLOCK * blockP = createBlock();
		//在原来文件夹的间接扇区上增加新增的扇区编号
		inodeMemAddBlock(inodeP, blockP->blockNumber);

		char * mem = (char *)Malloc(superBlockPointer->blockSize);
		readBlock(blockP, mem);
		mem[(inodeP->length+1) % superBlockPointer->blockSize] = c;
		free(mem);
		freeBlock(blockP);
		inodeP->length ++;
	}
	else{
		int blockNumber = getCurrentBlockNumber(inodeP);
		BLOCK * blockP = getBlock(blockNumber);
		char * mem = (char *)Malloc(superBlockPointer->blockSize);
		readBlock(blockP, mem);
		mem[(inodeP->length+1) % superBlockPointer->blockSize] = c;
		free(mem);
		freeBlock(blockP);
		inodeP->length ++;
	}
}

void writeINODEData(INODE * inodeP, char c, unsigned int offset)
{
	if(offset > inodeP->length){
		while(inodeP->length < offset){
			writeINODEData(inodeP, '\0');
		}
		writeINODEData(inodeP, c, offset);
	}
	else{
		int blockNumber = getBlockNumber(inodeP, offset);
		if(blockNumber == NULL){
			return ;
		}
		BLOCK * blockP = getBlock(blockNumber);
		char * mem = (char *)Malloc(superBlockPointer->blockSize);
		readBlock(blockP, mem);
		mem[offset % superBlockPointer->blockSize] = c;
		writeBlock(blockP, mem);
		free(mem);
		freeBlock(blockP);
	}
}

BOOL isDir(INODE * inodeP)
{
	if(NULL == inodeP){
		return FALSE;
	}
	if(inodeP->authority & _DIR_DEFINE_)
		return TRUE;
	else
		return FALSE;
}

BOOL isFile(INODE * inodeP)
{
	if(NULL == inodeP){
		return FALSE;
	}
	if(inodeP->authority & _FILE_DEFINE_){
		return TRUE;
	}
	else{
		return FALSE;
	}
}

void freeInode(INODE * inodeP)
{
	free(inodeP->mem);
	free(inodeP);
}

int getCurrentBlockNumber(INODE * inodeP)
{
	return ((int *)inodeP->mem)[inodeP->length / superBlockPointer->blockSize];
}

int getBlockNumber(INODE * inodeP, unsigned int offset)
{
	if(offset < inodeP->length){
		return NULL;
	}
	return ((int *)inodeP->mem)[offset / superBlockPointer->blockSize];
}

void inodeMemAddBlock(INODE * inodeP, int blockNumber)
{
	int * p = (int *)inodeP->mem;
	p[inodeP->length / superBlockPointer->blockSize] = blockNumber;
	//刷新磁盘
	writeINODE(inodeP);
}

void inodeDirAddFile(INODE * inodeP, void * mem, int length)
{
	if(length != 32){
		return ;
	}
	void * _mem = Malloc(superBlockPointer->blockSize);

	//当前扇区上添加新增的文件数据。 及文件名和文件的INODE编号
	BLOCK * blockP = getBlock(getCurrentBlockNumber(inodeP));
	readBlock(blockP, _mem);
	memcpy((char*)_mem + inodeP->length % superBlockPointer->blockSize, mem, 32);
	writeBlock(blockP, _mem);
	freeBlock(blockP);
	inodeP->length += 32;
	writeINODE(inodeP);
	free(_mem);
}

void writeNotInodeMetaData(INODE * inodeP , char c, unsigned int offset)
{

}

void printInode(INODE * inodeP)
{
	if(inodeP == NULL){
		return;
	}
	if(isDir(inodeP)){
		printf("d");
	}
	else{
		printf("-");
	}
	printAuthority(inodeP);
	printf("\t");
	struct tm * tmpTime ;
	//printf(ctime(&inodeP->cTime));
	//printf(ctime(&inodeP->aTime));
	//printf(ctime(&inodeP->mTime));
	tmpTime = localtime(&inodeP->cTime);
	printf("cTime:%02d-%02d\t", tmpTime->tm_min, tmpTime->tm_sec);
	tmpTime = localtime(&inodeP->aTime);
	printf("aTime:%02d-%02d\t", tmpTime->tm_min, tmpTime->tm_sec);
	//tmpTime = localtime(&inodeP->mTime);
	//printf("修改时间：%d-%d-%d\t", tmpTime->tm_year, tmpTime->tm_mon, tmpTime->tm_mday);
	printf("size:%d B\t", inodeP->length);
}

void printAuthority(INODE * inodeP)
{
	printUserAuthority(inodeP);
	printGroupAuthority(inodeP);
	printOtherAuthority(inodeP);
}

void printUserAuthority(INODE * inodeP)
{
	if(inodeP->authority & _USER_READ_DEFINE_){
		printf("r");
	}
	else{
		printf("-");
	}
	if(inodeP->authority & _USER_WRITE_DEFINE_){
		printf("w");
	}
	else{
		printf("-");
	}
	if(inodeP->authority & _USER_EXEC_DEFINE_){
		printf("x");
	}
	else{
		printf("-");
	}
}

void printGroupAuthority(INODE * inodeP)
{
	if(inodeP->authority & _GROUP_READ_DEFINE_){
		printf("r");
	}
	else{
		printf("-");
	}
	if(inodeP->authority & _GROUP_WRITE_DEFINE_){
		printf("w");
	}
	else{
		printf("-");
	}
	if(inodeP->authority & _GROUP_EXEC_DEFINE_){
		printf("x");
	}
	else{
		printf("-");
	}

}

void printOtherAuthority(INODE * inodeP)
{
	if(inodeP->authority & _OTHER_READ_DEFINE_){
		printf("r");
	}
	else{
		printf("-");
	}
	if(inodeP->authority & _OTHER_WRITE_DEFINE_){
		printf("w");
	}
	else{
		printf("-");
	}
	if(inodeP->authority & _OTHER_EXEC_DEFINE_){
		printf("x");
	}
	else{
		printf("-");
	}
}