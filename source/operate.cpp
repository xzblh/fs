#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tool.h"
#include "superBlock.h"
#include "file.h"
#include "operate.h"

extern User * currentUser;
extern SUPER_BLOCK * superBlockPointer;
extern char * currentPwd;

void CD(char ** cmds) //get into foler
{
	//先实现最简单的使用
	//使用形式 必须是，小写：cd folder

}
void TOUCH(char ** cmds) //create file
{

}
void RM(char ** cmds) //remove file
{

}
void MKDIR(char ** cmds) //create folder
{

}
void RMDIR(char ** cmds) //remove folder
{

}
void LS(char ** cmds) //list files attributes
{
	FILE_FS * fileFsP = openFile(currentPwd);
	if(NULL ==fileFsP){
		printf("open current dir failed!\r\n");
		return;
	}
	int pos = 0;
	char str[32];
	char * mem = (char *)Malloc(fileFsP->inodeP->length);
	unsigned int length = getContent_FS(fileFsP, mem, fileFsP->inodeP->length);
	if(length != fileFsP->inodeP->length){
		length = (fileFsP->inodeP->length / superBlockPointer->blockSize)
						* superBlockPointer->blockSize;
	}
	int inodeNumber = 0;
	INODE * tmpInode;
	while(pos < length){
		memcpy(str, mem + pos, 32);
		inodeNumber = *(int *)(str + 28);
		tmpInode = getINODE(inodeNumber);
		printInode(tmpInode);
		printf("%s", str);
		printf("\r\n");
		freeInode(tmpInode);
		pos += 32;
	}
}
void WRITE(char ** cmds) //write file
{

}
void READ(char ** cmds) //read file
{

}
void UMASK(char ** cmds) //set the file or folder's attributes
{

}