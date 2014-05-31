#ifndef _BLOCK_HEADER_
#define _BLOCK_HEADER_
	typedef struct _block{
		int blockNumber; //区块编号，当做偏移使用
		//
	}BLOCK;

#endif

void initBlock(BLOCK * blockP); //往扇区写0
void writeBlock(BLOCK * blockP, void * mem);
void readBlock(BLOCK * blockP, void * mem);
BLOCK * createBlock();
BLOCK * getBlock(int blockNumber);
int getBlockOffset(int blockNumber);
void freeBlock(BLOCK * blockP);