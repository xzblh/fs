#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "superBlock.h"
#include "INODE.h"
#include "tool.h"


extern FILE * dataFp;
extern SUPER_BLOCK * superBlockPointer;
extern User currentUser;

INODE * createINODE()
{
	int inodePos = findZero(superBlockPointer->iBitMap, superBlockPointer->inodeBitMapCount * superBlockPointer->blockSize / 8);
	INODE * inodeP = (INODE*)Malloc(sizeof(INODE));
	inodeP->inodeNumber = inodePos;
	inodeP->GID = currentUser.GID;
	inodeP->UID = currentUser.UID;
	//inodeP->cTime = now(); //当前时间
	//inodeP->mTime = now(); //修改时间
	//inodeP->aTime = now(); //访问时间
	inodeP->authority  = _755_AUTHORITY_DIR_; // 默认drwxr-xr-x 755
	int blockPos = findZero(superBlockPointer->bBitMap, superBlockPointer->blockBitMapCount * superBlockPointer->blockSize / 8);
	inodeP->blockNumber = blockPos;
	inodeP->length = 0;
	return inodeP;
}

void writeINODE(INODE * inodeP)
{
	//data.txt的区块偏移， 加上INODE节点区的偏移
	int offset = inodeP->inodeNumber * sizeof(INODE) + 17 * superBlockPointer->blockSize;
	if(fseek(dataFp, offset, SEEK_SET) == 0){
		fwrite(inodeP, sizeof(INODE), 1, dataFp);
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
	if(inodeP->authority && _DIR_DEFINE)
		return TRUE;
	else
		return FALSE;
}
