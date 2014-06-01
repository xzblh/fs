#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "superBlock.h"
#include "file.h"
#include "tool.h"

extern char * currentPwd;
extern User * currentUser;
extern FILE * dataFp;
extern SUPER_BLOCK * superBlockPointer;


int getc_FS(FILE_FS * fileFsP)
{
	//偏移为1，长度为2。则偏移为2时应该没有数据
	if(fileFsP->offset >= fileFsP->inodeP->length){
		return -1;
	}
	int pos = fileFsP->offset % superBlockPointer->blockSize;
	char * s = (char*)fileFsP->mem;
	int ch = s[pos];
	if(fileFsP->offset < fileFsP->inodeP->length - 1){
		fileFsP->offset++;
	}
	if(fileFsP->offset % superBlockPointer->blockSize == 0){
		readFileContent(fileFsP);
	}
	return ch;
}

unsigned int getContent_FS(FILE_FS * fileFsP, void * mem, unsigned int length)
{
	if(fileFsP == NULL || mem == NULL){
		return NULL;
	}
	char * str = (char*)mem;
	unsigned int i = 0;
	char c = NULL;
	while(i < length){
		c = getc_FS(fileFsP);
		if(c == -1){
			return i;
		}
		str[i++] = c;
	}
	return length;
}

int writeContent(INODE * inodeP, void * mem, int length, int offset)
{
	if(length + offset >= getFileSizeLimit(superBlockPointer)){
		//printf("超出单文件大小限！\r\n");
		return -1;
	}
	int l = ftell(dataFp);
	fseek(dataFp, inodeP->blockNumber * superBlockPointer->blockSize, SEEK_SET);
	Fwrite(mem, length, 1, dataFp);
	fseek(dataFp, l, SEEK_SET);
	return 0;
}

int writeFileContent(FILE_FS * fileFsP) //刷新缓存到扇区  类似于正常文件系统中的磁盘同步操作
										//还应该进行文件大小检查等操作
{
	int blockNumber = getCurrentBlockNumber(fileFsP);
	BLOCK * blockP = NULL;
	if(NULL == blockNumber){
		blockNumber = getFreeBlockNumber(superBlockPointer);
		blockP = getBlock(blockNumber);
		//INODE增加分配的扇区的记录
		inodeMemAddBlock(fileFsP->inodeP, blockNumber);
		initBlock(blockP);
	}
	else{
		blockP = getBlock(blockNumber);
	}
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
		//INODE增加分配的扇区的记录
		inodeMemAddBlock(fileFsP->inodeP, blockNumber);
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
	if(inodeP->length % superBlockPointer->blockSize == 0){
		//之前分配的block刚好用完。
		if(inodeP->length + (unsigned int)32 >= getFileSizeLimit(superBlockPointer)){
			//单文件大小限制
			return -3;
		}
		else{
			//获得为原来文件夹新增的扇区
			BLOCK * blockP = createBlock();
			//在原来文件夹的间接扇区上增加新增的扇区编号
			inodeMemAddBlock(inodeP, blockP->blockNumber);
			freeBlock(blockP);
		}
	}

	//获得新文件的INODE
	INODE * tmpInodePointer = createINODE(_755_AUTHORITY_FILE_);

	//把文件名和文件的INODE编号组合在32个字节里面。
	char str[32];
	memset(str, 0, 32);
	strcpy(str, fileName);
	*(unsigned int*)(str+28) = tmpInodePointer->inodeNumber;
	inodeDirAddFile(inodeP, str, 32);

	writeINODE(tmpInodePointer);
	return 0;
}

int createDir(INODE * inodeP, char * dirName)
{
	if(strlen(dirName) > 27){
		//文件夹名长度限制 必须小于等于27，最长不能超过27
		return -4;
	}
	if(!hasCreateFileAuthority(inodeP, currentUser)){
		return -2;
	}
	if(!isDir(inodeP)){
		return -1;
	}

	if(inodeP->length % superBlockPointer->blockSize == 0){
		//之前分配的block刚好用完。
		if(inodeP->length + (unsigned int)32 >= getFileSizeLimit(superBlockPointer)){
			//单文件大小限制
			return -3;
		}
		else{
			//获得为原来文件夹新增的扇区
			BLOCK * blockP = createBlock();
			//在原来文件夹的间接扇区上增加新增的扇区编号
			inodeMemAddBlock(inodeP, blockP->blockNumber);
			freeBlock(blockP);
		}
	}
	INODE * newDirInodeP = createINODE(_755_AUTHORITY_DIR_);

	char str[32];

	//把文件名和文件的INODE编号组合在32个字节里面。
	memset(str, 0, 32);
	strcpy(str, dirName);
	*(unsigned int*)(str+28) = newDirInodeP->inodeNumber;
	inodeDirAddFile(inodeP, str, 32);

	//添加父文件夹节点到当前文件夹
	memset(str, 0, 32);
	strcpy(str, "..");
	*(unsigned int*)(str+28) = inodeP->inodeNumber;
	inodeDirAddFile(newDirInodeP, str, 32);

	//添加当前文件节点到当前文件夹
	memset(str, 0, 32);
	strcpy(str, ".");
	*(unsigned int*)(str+28) = newDirInodeP->inodeNumber;
	inodeDirAddFile(newDirInodeP, str, 32);

	writeINODE(inodeP); //这个可以不用调用的，因为inodeDirAddFile()函数已经调用过了
	writeINODE(newDirInodeP);
	return 0;

}

void writeAddUser(User * userP, FILE_FS * fileFsP)
{
	char str[512];
	memset(str, 0, 512);
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
	sscanf(buff, "%d %d %s %s\r\n", &userP->UID, &userP->GID, userP->username, userP->passwd);
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

FILE_FS * createFILE_FS(INODE* inodeP)
{
	if(inodeP == NULL){
		return NULL;
	}
	FILE_FS * fileFsP = (FILE_FS *)Malloc(sizeof(FILE_FS));
	fileFsP->mem = Malloc(superBlockPointer->blockSize);
	fileFsP->inodeP = inodeP;
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
		if(c != '\r' && c != '\n'){
			str[i++] = c;
		}
		else{
			str[i++] = '\r';
			str[i++] = '\n';
			str[i++] = '\0';
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

int fseekFs(FILE_FS * fileFsP, unsigned int offset)
{
	if(offset > fileFsP->inodeP->length){
		return -1;
	}
	fileFsP->offset = offset;
	readFileContent(fileFsP);
}


int getFileInodeInFolder(FILE_FS * fileFsP, char * fileName)
{
	if(fileFsP == NULL){
		return NULL;
	}
	char str[32];
	memset(str, 0, 32);
	//int pos = 0;
	fseekFs(fileFsP, 0);
	while(getContent_FS(fileFsP, str, 32)  == 32){
		if(strcmp(str, fileName) == 0){
			return *(int *)(str+28);
		}
	}
	return NULL;
	/*
	char * src = (char *)fileFsP->mem;
	while(pos < fileFsP->inodeP->length){
	memcpy(str, src + (pos%superBlockPointer->blockSize), 32);
	pos += 32;
	fileFsP->offset += 32;
	if(fileFsP->offset % superBlockPointer->blockSize == 0){
	readFileContent(fileFsP);
	}
	}
	return NULL;*/
}