#include <stdio.h>
#include <time.h>

#include "INODE.h"

#ifndef _SUPERBLOCK_HEADER_
#define _SUPERBLOCK_HEADER_
	typedef struct _superblock{
		unsigned int blockCount;
		unsigned int inodeCount;
		unsigned int blockFreeCount;
		unsigned int inodeFreeCount;
		unsigned int blockSize; //对于本测试程序来说，固定512字节
		unsigned int inodeSize; //对于本测试程序来说，固定xxx字节(该值尚未确定)
		unsigned int blockBitMapCount; //block的bitmap占用了多少扇区  固定8个扇区
		unsigned int blockBitMapStart; //block的bitmap的起始扇区
		unsigned int inodeBitMapCount; //inode的bitmap占用了多少扇区  固定8个扇区
		unsigned int inodeBitMapStart; //inode的bitmap的起始扇区
		time_t mountTime; //挂载时间
		INODE * inode;  //文件系统根节点
		void * bBitMap; //存储blockBitMap数据
		void * iBitMap; //存储inodeBitMap数据
	}SUPER_BLOCK;
#endif

void writeSuperBlock(SUPER_BLOCK * superBlockP, FILE * fp);
void writeBitMap(SUPER_BLOCK * superBlockP, FILE * fp);
void readBitMap(SUPER_BLOCK * superBlockP, FILE * fp);
void writeRoot(SUPER_BLOCK * superBlockP);
void readRoot(SUPER_BLOCK * superBlockP);
unsigned int getFileSizeLimit(SUPER_BLOCK * superBlockP);
int getInodeAreaOffset(SUPER_BLOCK * superBlockP);
unsigned int getFreeBlockNumber(SUPER_BLOCK * superBlockP);
unsigned int getFreeInodeNumber(SUPER_BLOCK * superBlockP);
unsigned int getBlockBitMapByteCount(SUPER_BLOCK * superBlockP);
unsigned int getInodeBitMapByteCount(SUPER_BLOCK * superBlockP);
