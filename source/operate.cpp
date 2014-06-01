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
	if(NULL == cmds[1]){
		printf("params number is not right!\r\n");
		return;
	}
	else{
		FILE_FS *fileFsP = openFile(currentPwd);
		if(getFileInodeInFolder(fileFsP, cmds[1])){
			printf("%s is exist!\r\n", cmds[1]);
		}
		int result = createFile(fileFsP->inodeP, cmds[1]);
		switch(result){
		case -1:
			printf("%s is not a directory!\r\n", currentPwd);
			break;
		case -2:
			printf("Permission denied!\r\n");
			break;
		case -3:
			printf("You have too much file or dir in %s!\r\n", currentPwd);
			break;
		case -4:
			printf("%s is too long for as fileName!\r\n", cmds[1]);
			break;
		case -5:
			printf("can not contain with /\r\n");
			break;
		default:
			printf("Success!\r\n");
		}
	}
}

void RM(char ** cmds) //remove file
{

}

void MKDIR(char ** cmds) //create folder
{
	if(NULL == cmds[1]){
		printf("params number is not right!\r\n");
		return;
	}
	else{
		FILE_FS *fileFsP = openFile(currentPwd);
		if(getFileInodeInFolder(fileFsP, cmds[1])){
			printf("%s is exist!\r\n", cmds[1]);
		}
		else{
			int result = createDir(fileFsP->inodeP, cmds[1]);
			switch(result){
			case -1:
				printf("%s is not a directory!\r\n", currentPwd);
				break;
			case -2:
				printf("Permission denied!\r\n");
				break;
			case -3:
				printf("You have too much file or dir in %s!\r\n", currentPwd);
				break;
			case -4:
				printf("%s is too long for as fileName!\r\n", cmds[1]);
				break;
			case -5:
				printf("can not contain with /\r\n");
				break;
			default:
				printf("Success!\r\n");
			}
		}
		freeFILE_FS(fileFsP);
	}

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
		printf("%d\t", tmpInode->inodeNumber);
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
	if(NULL == cmds[1]){
		printf("params number is not right!\r\n");
		return ;
	}
	char path[256];
	strcpy(path, currentPwd);
	strcat(path, cmds[1]);
	FILE_FS * fileFsP = openFile(path);
	if(NULL == fileFsP){
		printf("no such file!\r\n");
	}
	else{
		char str[32];
		memset(str, 0, 32);
		fseekFs(fileFsP, 0);
		int ch = NULL;
		while(fileFsP->offset < fileFsP->inodeP->length){
			ch = getc_FS(fileFsP);
			if(-1 != ch){
				printf("%c", ch);
			}
		}
		printf("\r\n");
	}

}

void UMASK(char ** cmds) //set the file or folder's attributes
{

}