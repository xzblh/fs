#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tool.h"
#include "superBlock.h"
#include "file.h"

extern User * currentUser;

void writeSuperBlock(SUPER_BLOCK * superBlockP, FILE * fp)
{
	int result = fseek(fp, 512, SEEK_SET);
	int l = sizeof(time_t);
	if(result == 0){
		//fwrite(&(superBlockP->blockCount), 4, 1, dataFp);
		fwrite(superBlockP, sizeof(SUPER_BLOCK) - 8, 1, fp);
		fflush(fp);
		/*fprintf(dataFp, "%d", superBlockP->blockCount);*/
	}
	else if(result == 0){
		perror("文件偏移出错");
		exit(-1);
	}
}


void writeBitMap(SUPER_BLOCK * superBlockP, FILE * fp)
{
	fseek(fp, 512*2, SEEK_SET);
	fwrite(superBlockP->bBitMap, superBlockP->blockSize / 8, superBlockP->blockBitMapCount, fp);
	fwrite(superBlockP->iBitMap, superBlockP->blockSize / 8, superBlockP->inodeBitMapCount, fp);
}

void readBitMap(SUPER_BLOCK * superBlockP, FILE * fp)
{
	fseek(fp, 512*2, SEEK_SET);
	fread(superBlockP->bBitMap, superBlockP->blockSize / 8, superBlockP->blockBitMapCount, fp);
	superBlockP->blockFreeCount = superBlockP->blockCount - countMem(superBlockP->bBitMap, 
		superBlockP->blockBitMapCount * superBlockP->blockSize);
	fread(superBlockP->iBitMap, superBlockP->blockSize / 8, superBlockP->inodeBitMapCount, fp);
	superBlockP->inodeFreeCount = superBlockP->inodeCount - countMem(superBlockP->iBitMap,
		superBlockP->inodeBitMapCount * superBlockP->blockSize);
}

void writeRoot(SUPER_BLOCK * superBlockP)
{
	//只有初始化时才会调用到这个文件，通常是调用下面的读方法
	//inodeP的inodeNumber就是0
	INODE * inodeP = createINODE(_755_AUTHORITY_DIR_); // 默认drwxr-xr-x 755
	writeINODE(inodeP);
	superBlockP->inode = inodeP;
}

void readRoot(SUPER_BLOCK * superBlockP)
{
	INODE * inodeP = getINODE(0);//根节点的Inode号就是0
	superBlockP->inode = inodeP;
}

unsigned int getFileSizeLimit(SUPER_BLOCK * superBlockP)
{
	if(NULL == superBlockP){
		return 0;
	}
	return superBlockP->blockSize * (superBlockP->blockSize / 4);
}


int getInodeAreaOffset(SUPER_BLOCK * superBlockP)
{
	//data.txt文件至少包含一个启动块，一个超级块，一个blockBitMap，即8块，一个InodeBitMap， 即8块。
	//一个Inode节点区，该区大概8*512*32字节，即256个扇区。总共 1+1+8+8+256 = 274
	return superBlockP->blockSize * (1 + 1 + superBlockP->blockBitMapCount + superBlockP->inodeBitMapCount);
}

unsigned int getFreeBlockNumber(SUPER_BLOCK * superBlockP)
{
	return findZero(superBlockP->bBitMap, getBlockBitMapByteCount(superBlockP));
}

unsigned int getFreeInodeNumber(SUPER_BLOCK * superBlockP)
{
	return findZero(superBlockP->iBitMap, getInodeBitMapByteCount(superBlockP));
}

unsigned int getBlockBitMapByteCount(SUPER_BLOCK * superBlockP)
{
	return superBlockP->blockBitMapCount * superBlockP->blockSize / 8;
}

unsigned int getInodeBitMapByteCount(SUPER_BLOCK * superBlockP)
{
	return superBlockP->inodeBitMapCount * superBlockP->blockSize / 8;
}