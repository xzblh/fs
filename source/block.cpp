#include <stdio.h>

#include "block.h"
#include "superBlock.h"

extern SUPER_BLOCK * superBlockPointer;
extern FILE * dataFp;

void writeBlock(BLOCK * blockP, void * mem)
{
	if(NULL == mem){
		return ;
	}
	fseek(dataFp, blockP->blockNumber * superBlockPointer->blockSize, SEEK_SET);
	fwrite(mem, superBlockPointer->blockSize, 1, dataFp);
}

//找到一个没用的扇区，并全部写0，返回该扇区的编号
BLOCK * createBlock()
{
	int blockPos = findZero(superBlockPointer->bBitMap, 
		superBlockPointer->blockBitMapCount * superBlockPointer->blockSize / 8);
	fseek(dataFp, superBlockPointer->blockSize * blockPos, SEEK_SET);
	writeNull(512, dataFp);
	BLOCK * blockP = (BLOCK *)Malloc(sizeof(BLOCK));
	blockP->blockNumber = blockPos;
	return blockP;
}

//根据扇区编号获得扇区指针
BLOCK * getBlock(int blockNumber)
{
	BLOCK * blockP = (BLOCK *)Malloc(sizeof(BLOCK));
	return blockP;
}

int getBlockOffset(int blockNumber)
{
	return superBlockPointer->blockSize * blockNumber;
}