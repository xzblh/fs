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
					if(!hasGetIntoAuthority(tmpInode, currentUser)){
						printf("Permission denied!\r\n");
					}
					else{
						pathCat(currentPwd, cmds[1]);
						printf("Current Dir:%s\r\n", currentPwd);
					}
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
			return;
		}
		int result = createFile(fileFsP->inodeP, cmds[1], ~currentUser->umask & _664_AUTHORITY_FILE_);
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
	if(cmds[1] == NULL){
		return ;
	}
	else{
		FILE_FS * fieFsP = openFile(currentPwd);
		if(fieFsP == NULL){
			printf("No such file:%s\r\n", cmds[1]);
			return;
		}
		if(!hasWriteAuthority(fieFsP->inodeP, currentUser)){
			printf("Permission denied!\r\n");
		}
		else{
			removeFile(fieFsP->inodeP, cmds[1]);
		}
	}
}

void MKDIR(char ** cmds) //create folder
{
	if(NULL == cmds[1]){
		printf("params number is not right!\r\n");
		return;
	}
	else{
		FILE_FS *fileFsP = openFile(currentPwd);
		if(NULL == fileFsP){
			printf("Cannot open directory:%s\r\n", currentPwd);
		}
		else if(!hasWriteAuthority(fileFsP->inodeP, currentUser)){
			printf("Permission denied!\r\n");
		}

		if(getFileInodeInFolder(fileFsP, cmds[1])){
			printf("%s is exist!\r\n", cmds[1]);
		}
		else{
			int result = createDir(fileFsP->inodeP, cmds[1], ~currentUser->umask & _755_AUTHORITY_DIR_);
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
	if(cmds[1] == NULL){
		return ;
	}
	else{
		FILE_FS * fileFsP = openFile(currentPwd);
		if(NULL == fileFsP){
			printf("Cannot open directory:%s\r\n", currentPwd);
		}
		else if(!hasWriteAuthority(fileFsP->inodeP, currentUser)){
			printf("Permission denied!\r\n");
		}
		else{
			removeDir(fileFsP->inodeP, cmds[1]);
		}
	}
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
		pos += 32;
		if(inodeNumber != 0){
			tmpInode = getINODE(inodeNumber);
		}
		else{
			if(strcmp(str, ".") == 0 || strcmp(str, "..") == 0){
				tmpInode = getINODE(inodeNumber);
			}
			else{
				continue;
			}
		}
		printf("%d\t", tmpInode->inodeNumber);
		printInode(tmpInode);
		printf("%s", str);
		printf("\r\n");
		freeInode(tmpInode);
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
		FILE_FS * currentDir = openFile(currentPwd);
		if(getFileInodeInFolder(currentDir, cmds[1]) == 0){
			freeFILE_FS(currentDir);
			printf("No such File!\r\n");
			return;
		}
		free(currentDir->mem);
		free(currentDir);
		FILE_FS * fileFsP = openFile(cmds[1]);
		if(fileFsP == NULL || !isFile(fileFsP->inodeP)){
			printf("%s is not a file!\r\n");
			return ;
		}
		if(!hasWriteAuthority(fileFsP->inodeP, currentUser)){
			printf("Permission denied!\r\n");
			return;
		}
		fseekFs(fileFsP, fileFsP->inodeP->length);
		printf("input the file content!\r\n");
		char * mem = (char *)Malloc(1024);
		scanf("%1000[^\n]", mem);
		time(&fileFsP->inodeP->aTime);
		time(&fileFsP->inodeP->mTime);
		writeINODE(fileFsP->inodeP);
		writeFileBuffer(fileFsP, mem);
		free(mem);
	}
}

void READ(char ** cmds) //read file
{
	if(NULL == cmds[1]){
		printf("params number is not right!\r\n");
		return ;
	}
	char path[256];
	int length = strlen(currentPwd);
	strcpy(path, currentPwd);
	if(currentPwd[length-1] != '/'){
		strcat(path, "/");
	}
	strcat(path, cmds[1]);
	FILE_FS * fileFsP = openFile(path);
	if(NULL == fileFsP){
		printf("no such file!\r\n");
	}
	else if(!isFile(fileFsP->inodeP)){
		printf("%s is not a file!\r\n", cmds[1]);
	}
	else if(!hasReadAuthority(fileFsP->inodeP, currentUser)){
		printf("Permission denied!\r\n");
		return;
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
		void * pppp = Malloc(10240);
		time(&fileFsP->inodeP->aTime);
		writeINODE(fileFsP->inodeP);
		printf("\r\n");
	}

}

void UMASK(char ** cmds) //set the file or folder's attributes
{
	if(cmds == NULL){
		printf("Params number is not right!\r\n");
		return;
	}
	else if(cmds[1] == NULL || strcmp(cmds[1], "-l") == 0 ){
		printf("%d%d%d\r\n", currentUser->umask/8, currentUser->umask/8/8, currentUser->umask%8);
	}
	else{
		setUmask(currentUser, cmds[1]);
	}
}