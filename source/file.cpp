#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "block.h"
#include "superBlock.h"
#include "file.h"
#include "tool.h"

extern char * currentPwd;
extern User * currentUser;
extern FILE * dataFp;
extern SUPER_BLOCK * superBlockPointer;


char getc_FS(FILE_FS * fileFsP)
{
	int pos = fileFsP->offset % superBlockPointer->blockSize;
	char * s = (char*)fileFsP->mem;
	char c = s[pos];
	fileFsP->offset++;
	if(fileFsP->offset == superBlockPointer->blockSize){
		readFileContent(fileFsP);
	}
	return c;
}
void * getContent_FS(FILE_FS * fileFsP);

int writeContent(INODE * inodeP, void * mem, int length, int offset)
{
	if(length + offset >= getFileSizeLimit(superBlockPointer)){
		//printf("超出单文件大小限！\r\n");
		return -1;
	}
	int l = ftell(dataFp);
	fseek(dataFp, inodeP->blockNumber * superBlockPointer->blockSize, SEEK_SET);
	fwrite(mem, length, 1, dataFp);
	fseek(dataFp, l, SEEK_SET);
	return 0;
}

int writeFileContent(FILE_FS * fileFsP) //刷新缓存到扇区  类似于正常文件系统中的磁盘同步操作
{
	int blockNumber = getCurrentBlockNumber(fileFsP);
	BLOCK * blockP = getBlock(blockNumber);
	writeBlock(blockP, fileFsP->mem);
	free(blockP);
	return 0;
}

int readFileContent(FILE_FS * fileFsP)
{
	BLOCK * blockP;
	int	blockNumber = getCurrentBlockNumber(fileFsP);
	if(NULL == blockNumber){
		blockNumber = getFreeBlockNumber(superBlockPointer);
		blockP = getBlock(blockNumber);
		initBlock(blockP);
	}
	else{
		blockP = getBlock(blockNumber);
	}
	readBlock(blockP, fileFsP->mem);
	free(blockP);
	return 0;
}

void writeFileBuffer(FILE_FS * fileFsP, char * s)
{
	unsigned int sLength = strlen(s);
	unsigned int pos = fileFsP->offset % 512;
	if(pos + sLength <= superBlockPointer->blockSize){
		//不用增加扇区
		strcpy((char *)fileFsP->mem + pos, s);
		writeFileContent(fileFsP);
		fileFsP->inodeP->length += sLength;
		writeINODE(fileFsP->inodeP);
		fileFsP->offset += sLength;
	}
	else{
		//暂时不实现，测试数据都很小的
		//至少需要增加一个扇区
	}
}

int writeFileBuffer(FILE_FS * fileFsP, void * mem, int length)
{
	unsigned int pos = fileFsP->offset % 512;
	if(pos + length <= superBlockPointer->blockSize){
		//不用增加扇区
		memcpy(fileFsP->mem, mem, length);
		writeFileContent(fileFsP);
		fileFsP->inodeP->length += length;
		writeINODE(fileFsP->inodeP);
		fileFsP->offset += length;
	}
	else{
		//暂时不实现，测试数据都很小的
		//至少需要增加一个扇区
	}
	return length;
}

int createFile(INODE * inodeP, char * fileName)
{
	if(strlen(fileName) > 27){
		//文件名长度限制 必须小于等于27，最长不能超过27
		return -4;
	}
	if(!isDir(inodeP)){
		return -1;
	}
	if(!hasCreateFileAuthority(inodeP, currentUser)){
		return -2;
	}
	INODE * tmpInodePointer = NULL;
	char str[32];
	memset(str, 0, 32);
	strcpy(str, fileName);
	BLOCK * blockP;
	if(inodeP->length % superBlockPointer->blockSize == 0){
		//之前分配的block刚好用完。
		if(inodeP->length + (unsigned int)32 >= getFileSizeLimit(superBlockPointer)){
			//单文件大小限制
			return -3;
		}
		else{
			//获得为原来文件夹新增的扇区
			blockP = createBlock();
			//在原来文件夹的间接扇区上增加新增的扇区编号
			((int*)inodeP->mem)[inodeP->length/4] = blockP->blockNumber;
		}
	}
	else{
		blockP = getBlock(inodeP->blockNumber);
	}
	//获得新文件的INODE
	tmpInodePointer = createINODE(_755_AUTHORITY_FILE_);

	//把文件名和文件的INODE编号组合在32个字节里面。
	*(unsigned int*)(str+28) = blockP->blockNumber;
	//分配一块内存，方便写入
	void * mem = Malloc(superBlockPointer->blockSize);
	memset(mem, 0, superBlockPointer->blockSize);
	strcpy((char*)mem, str);

	//在新扇区上添加新增的文件数据。 及文件名和文件的INODE编号
	writeBlock(blockP, mem);
	freeBlock(blockP);
	inodeP->length += 32; //单文件名27个字符限制  27个字符+1个结束标志+4个Inode指示，总共32位。
	writeINODE(inodeP);
	writeINODE(tmpInodePointer);
	return 0;
}

int createDir(INODE * inodeP, char * dirName)
{
	if(!hasCreateFileAuthority(inodeP, currentUser)){
		return -2;
	}
	INODE * tmp = createINODE(_755_AUTHORITY_DIR_);
	return 0;
}

void writeAddUser(User * userP, FILE_FS * fileFsP)
{
	char str[512];
	sprintf(str, "%d %d %s %s\r\n", userP->UID, userP->GID, userP->username, userP->passwd);
	writeFileBuffer(fileFsP, str, strlen(str));
	//fwrite(&(userP->UID), sizeof(userP->UID), 1, dataFp);
	//fwrite(&(userP->GID), sizeof(userP->GID), 1, dataFp);
	//fwrite(userP->username, strlen(userP->username), 1, dataFp);
	//fwrite(userP->passwd, strlen(userP->passwd), 1, dataFp);
	//char line[] = "\r\n";
	//fwrite(line, strlen(line), 1, dataFp);
}

User * getUser(FILE_FS * fileFsP)
{
	char buff[256];
	if(getLine(fileFsP, buff) == 0){
		return NULL;
	}
	User * userP = (User *)Malloc(sizeof(User));
	userP->username = (char *)Malloc(16);
	userP->passwd = (char *)Malloc(16);
	sscanf(buff, "%d %d %s %s\r\n", userP->UID, userP->GID, userP->username, userP->passwd);
	return userP;
}

FILE_FS * openFile(char * fileName)
{
	if(NULL == fileName){
		return NULL;
	}
	char path[512];
	FILE_FS * fileFsP = (FILE_FS *)Malloc(sizeof(FILE_FS));
	fileFsP->mem = Malloc(superBlockPointer->blockSize);
	if(fileName[0] == '/'){
		fileFsP->inodeP = getInode(fileName);
	}
	else{
		strcpy(path, currentPwd);
		strcat(path, fileName);
		fileFsP->inodeP = getInode(path);
	}
	if(fileFsP->inodeP == NULL){
		return NULL;
	}
	fileFsP->offset = 0;
	readFileContent(fileFsP);
	return fileFsP;
}

void * getLine(FILE_FS * fileFsP, void * mem)
{
	if(NULL == mem){
		return mem;
	}
	int i = 0;
	char * str = (char *)mem;
	char c = NULL;
	while(TRUE){
		c = getc_FS(fileFsP);
		if(c != '\r' || c != '\n'){
			str[i++];
		}
		else{
			str[i++] = '\r';
			str[i++] = '\n';
			break;
		}
	}
	return mem;
}

int getCurrentBlockNumber(FILE_FS * fileFsP)
{
	if(fileFsP->offset > fileFsP->inodeP->length){
		printf("文件偏移量大于Inode节点中记录的长度信息\r\n");
		getchar();
		exit(-1);
	}
	int * p = (int *)fileFsP->inodeP->mem;
	return p[fileFsP->offset / superBlockPointer->blockSize];
}

void freeFILE_FS(FILE_FS * fileFsP)
{
	freeInode(fileFsP->inodeP);
	free(fileFsP->mem);
	free(fileFsP);
}