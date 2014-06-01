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
	if(cmds == NULL){
		printf("params number is not right!\r\n");
		return;
	}
	else{
		if(strcmp(cmds[1], ".") == 0){
			printf("current work directory:%s\r\n", currentPwd);
			return;
		}
		else if(strcmp(cmds[1], "..") == 0){
			char * pos = strrchr(currentPwd, '/');
			if(NULL == pos || pos == currentPwd){
				currentPwd[0] = '/';
				currentPwd[1] = '\0';
			}
			else{
				*pos = '\0';
			}
			printf("current work directory:%s\r\n", currentPwd);
			return;
		}
		else{
			//更一般的操作
			FILE_FS * fileFsP = openFile(currentPwd);
			int inodeNumber = 0;
			inodeNumber = getFileInodeInFolder(fileFsP, cmds[1]);
			if(inodeNumber != 0){
				INODE * tmpInode = getINODE(inodeNumber);
				if(isDir(tmpInode) ){
					strcat(currentPwd, cmds[1]);
					printf("Current Dir:%s\r\n", currentPwd);
				}
				else{
					freeInode(tmpInode);
					printf("%s is not a directory!\r\n", cmds[1]);
				}
			}
			else{
				printf("No such Directory!\r\n");
			}
			free(fileFsP->mem);
			free(fileFsP);
		}
	}
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
		if(fileFsP->inodeP != superBlockPointer->inode){
			freeFILE_FS(fileFsP);
		}
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
	if(fileFsP->inodeP != superBlockPointer->inode){
		freeFILE_FS(fileFsP);
	}
}

void WRITE(char ** cmds) //write file
{
	if(NULL == cmds[1]){
		printf("Nothing to write!\r\n");
		return;
	}
	else{
		if(getFileInodeInFolder(cmds[1]) == 0){
			printf("No such File!");
			return;
		}
		FILE_FS * fileFsP = openFile(cmds[1]);
		if(!isFile(fileFsP->inodeP)){
			printf("%s is not a file!\r\n");
			return ;
		}
		fseekFs(fileFsP, fileFsP->inodeP->length);
		printf("input the file content!");
		char * mem = (char *)Malloc(512);
		scanf("%512[^\n]", mem);
		writeFileBuffer(fileFsP, mem);
	}
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