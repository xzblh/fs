#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	int inodePos = findZero(superBlockPointer->iBitMap, superBlockPointer->inodeBitMapCount * superBlockPointer->blockSize / 8);
	INODE * inodeP = (INODE*)Malloc(sizeof(INODE));
	inodeP->inodeNumber = inodePos;
	inodeP->GID = currentUser->GID;
	inodeP->UID = currentUser->UID;
	//inodeP->cTime = now(); //当前时间
	//inodeP->mTime = now(); //修改时间
	//inodeP->aTime = now(); //访问时间
	inodeP->authority  = authority;

	BLOCK * blockP = createBlock();
	inodeP->blockNumber = blockP->blockNumber;
	free(blockP);
	inodeP->length = 0;
	return inodeP;
}

void writeINODE(INODE * inodeP)
{
	//data.txt的区块偏移， 加上INODE节点区的偏移
	int offset = inodeP->inodeNumber * sizeof(INODE) + 17 * superBlockPointer->blockSize;
	if(fseek(dataFp, offset, SEEK_SET) == 0){
		fwrite(&inodeP->aTime, 4, 1, dataFp);
		fwrite(&inodeP->cTime, 4, 1, dataFp);
		fwrite(&inodeP->mTime, 4, 1, dataFp);
		fwrite(&inodeP->GID, 4, 1, dataFp);
		fwrite(&inodeP->UID, 4, 1, dataFp);
		fwrite(&inodeP->authority, 4, 1, dataFp);
		fwrite(&inodeP->inodeNumber, 4, 1, dataFp);
		fwrite(&inodeP->blockNumber, 4, 1, dataFp);
		fwrite(&inodeP->length, 4, 1, dataFp);
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

void writeAddUser(User * userP, INODE * inodeP)
{
	writeUser(userP, inodeP->blockNumber * superBlockPointer->blockSize + inodeP->length);
}

void writeUser(User * userP, int offset)
{
	fseek(dataFp, offset, SEEK_SET);
	writeUser(userP);
}

void writeUser(User * userP)
{
	fwrite(&(userP->UID), sizeof(userP->UID), 1, dataFp);
	fwrite(&(userP->GID), sizeof(userP->GID), 1, dataFp);
	fwrite(userP->username, strlen(userP->username), 1, dataFp);
	fwrite(userP->passwd, strlen(userP->passwd), 1, dataFp);
	char line[] = "\r\n";
	fwrite(line, strlen(line), 1, dataFp);
}

BOOL isDir(INODE * inodeP)
{
	if(inodeP->authority && _DIR_DEFINE_)
		return TRUE;
	else
		return FALSE;
}
