#include <stdio.h>
#include <stdlib.h>

#include "tool.h"
#include "superBlock.h"

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
	fwrite(superBlockP->bBitMap, superBlockP->blockSize, superBlockP->blockBitMapCount, fp);
	fwrite(superBlockP->iBitMap, superBlockP->blockSize, superBlockP->inodeBitMapCount, fp);
}

void readBitMap(SUPER_BLOCK * superBlockP, FILE * fp)
{
	fseek(fp, 512*2, SEEK_SET);
	fread(superBlockP->bBitMap, superBlockP->blockSize, superBlockP->blockBitMapCount, fp);
	superBlockP->blockFreeCount = superBlockP->blockCount - countMem(superBlockP->bBitMap, 
		superBlockP->blockBitMapCount * superBlockP->blockSize);
	fread(superBlockP->iBitMap, superBlockP->blockSize, superBlockP->inodeBitMapCount, fp);
	superBlockP->inodeFreeCount = superBlockP->inodeCount - countMem(superBlockP->iBitMap,
		superBlockP->inodeBitMapCount * superBlockP->blockSize);
}