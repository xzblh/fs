#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
	if(fileFsP->offset < fileFsP->inodeP->length){
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
	fseekFs(fileFsP, fileFsP->inodeP->length); //移动当前光标到文件最后
	unsigned int sLength = strlen(s);
	unsigned int pos = fileFsP->offset % superBlockPointer->blockSize;
	unsigned int offset = pos + sLength;
	if(offset < (fileFsP->inodeP->length / superBlockPointer->blockSize + 1)
			*superBlockPointer->blockSize){
		//不用增加扇区
		strcpy((char *)fileFsP->mem + pos, s);
		writeFileContent(fileFsP);
		fileFsP->inodeP->length += sLength;
		writeINODE(fileFsP->inodeP);
		fileFsP->offset += sLength;
	}
	else{
		BLOCK * blockP = getBlock(getFreeBlockNumber(superBlockPointer));

		//写当前缓存能容纳的那部分数据
		memcpy((char *)fileFsP->mem + pos, s, superBlockPointer->blockSize - pos);
		writeFileContent(fileFsP);
		fileFsP->inodeP->length += superBlockPointer->blockSize - pos;
		writeINODE(fileFsP->inodeP);

		//复制剩下的部分中的一部分到内存，并写入磁盘
		memcpy((char *)fileFsP->mem, s + superBlockPointer->blockSize - pos,
			sLength - superBlockPointer->blockSize + pos);
		writeFileContent(fileFsP);
		fileFsP->inodeP->length += sLength - superBlockPointer->blockSize + pos;
		writeINODE(fileFsP->inodeP);

		//如果还没全部写入，递归调用继续写入磁盘
		if(sLength > superBlockPointer->blockSize){
			writeFileBuffer(fileFsP, s+superBlockPointer->blockSize);
		}
		//现在实现了……
		// blockP
		//暂时不实现，测试数据都很小的
		//至少需要增加一个扇区
	}
}

BOOL updateFileBuffer(FILE_FS * fileFsP, void * mem, unsigned int length, unsigned int pos)
{
	if(pos > superBlockPointer->blockSize || length > superBlockPointer->blockSize){
		return FALSE;
	}
	char * tMem = (char *)fileFsP->mem;
	memcpy(tMem + pos, mem, length);
	writeFileContent(fileFsP);
	return TRUE;
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

int createFile(INODE * inodeP, char * fileName, unsigned int authority)
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
	if(fileName == NULL || strchr(fileName, '/') != NULL){
		return -5;
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
	INODE * tmpInodePointer = createINODE(authority | _FILE_DEFINE_);

	//把文件名和文件的INODE编号组合在32个字节里面。
	char str[32];
	memset(str, 0, 32);
	strcpy(str, fileName);
	*(unsigned int*)(str+28) = tmpInodePointer->inodeNumber;
	inodeDirAddFile(inodeP, str, 32);

	writeINODE(tmpInodePointer);
	return 0;
}

int removeFile(INODE * inodeP)
{
	//释放所有扇区
	int number = NULL;
	int pos = 0;
	int * numberP = (int *)inodeP->mem;
	while(TRUE){
		number = numberP[pos++];
		if(number != NULL){
			setFreeBlockNumber(superBlockPointer, number);
		}
		else{
			break;
		}
	}
	setFreeBlockNumber(superBlockPointer, inodeP->blockNumber);
	//释放Inode节点
	setFreeInodeNumber(superBlockPointer, inodeP->inodeNumber);
	return 0;
}

int removeFile(INODE * inodeP, char * fileName)
{
	if(fileName == NULL){
		printf("param fileName is not valid!\r\n");
		return -4;
	}
	FILE_FS * fileFsP = openFile(fileName);
	if(fileFsP == NULL){
		printf("no such file:%s\r\n", fileName);
		return -3;
	}
	if(!isFile(fileFsP->inodeP)){
		freeFILE_FS(fileFsP);
		printf("%s is not a file!\r\n", fileName);
		return -1; //要删除的不是文件
	}
	//释放这个文件占用的inode和block编号
	removeFile(fileFsP->inodeP);

	//从inodeP这个文件夹里面删除fileName的记录
	char str[32];
	memset(str, 0, 32);
	FILE_FS * dirFsP = createFILE_FS(inodeP);
	fseekFs(dirFsP, 0);
	while(getContent_FS(dirFsP, str, 32)  == 32){
		if(strcmp(str, fileName) == 0 && *(int *)(str+28) == fileFsP->inodeP->inodeNumber){
			fseekFs(dirFsP, dirFsP->offset - 32);
			memset(str, 0, 32);
			updateFileBuffer(dirFsP, str, 32, dirFsP->offset % superBlockPointer->blockSize);
			freeFILE_FS(fileFsP);
			freeFILE_FS(dirFsP);
			return 0;
		}
	}
	freeFILE_FS(fileFsP);
	free(dirFsP->mem);
	free(dirFsP);
	return -2;
}

int createDir(INODE * inodeP, char * dirName, unsigned int authority)
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
	if(dirName == NULL || strchr(dirName, '/') != NULL){
		return -5;
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
	INODE * newDirInodeP = createINODE(authority | _755_AUTHORITY_DIR_); //确保创建的一定是个DIR节点

	char str[32];

	//把文件名和文件的INODE编号组合在32个字节里面。
	memset(str, 0, 32);
	strcpy(str, dirName);
	*(unsigned int*)(str+28) = newDirInodeP->inodeNumber;
	inodeDirAddFile(inodeP, str, 32);

	//为新文件夹分配间接扇区
	BLOCK * blockP = createBlock();
	//在原来文件夹的间接扇区上增加新增的扇区编号
	inodeMemAddBlock(newDirInodeP, blockP->blockNumber);
	freeBlock(blockP);

	//添加父文件夹节点到新文件夹
	memset(str, 0, 32);
	strcpy(str, "..");
	*(unsigned int*)(str+28) = inodeP->inodeNumber;
	inodeDirAddFile(newDirInodeP, str, 32);

	//添加当前文件节点到新文件夹
	memset(str, 0, 32);
	strcpy(str, ".");
	*(unsigned int*)(str+28) = newDirInodeP->inodeNumber;
	inodeDirAddFile(newDirInodeP, str, 32);

	writeINODE(inodeP); //这个可以不用调用的，因为inodeDirAddFile()函数已经调用过了
	writeINODE(newDirInodeP);
	return 0;

}

int removeRecordFromDir(FILE_FS * dirFsP, char * fileName)
{
	if(dirFsP == NULL || fileName == NULL || !isDir(dirFsP->inodeP)){
		return -1;
	}
	//从dirFsP这个文件夹里面删除fileName的记录
	char str[32];
	memset(str, 0, 32);
	fseekFs(dirFsP, 0);
	while(getContent_FS(dirFsP, str, 32)  == 32){
		if(strcmp(str, fileName) == 0 ){
			fseekFs(dirFsP, dirFsP->offset - 32);
			memset(str, 0, 32);
			updateFileBuffer(dirFsP, str, 32, dirFsP->offset % superBlockPointer->blockSize);
			return 0;
		}
	}
	return 0;
}

int removeDir(INODE * inodeP, char * dirName)
{
	if(dirName == NULL){
		printf("param is not valid!\r\n");
		return -5;
	}
	FILE_FS * delDirFsP = openFile(dirName);
	if(delDirFsP == NULL){
		printf("no such directory:%s!\r\n", dirName);
		return -4;
	}
	if(!isDir(delDirFsP->inodeP)){
		freeFILE_FS(delDirFsP);
		printf("%s is not a directory!\r\n", dirName);
		return -1; //要删除的不是一个目录
	}

	//保存工作目录
	char pwdStore[256];
	strcpy(pwdStore, currentPwd);

	char tmpStr[32];
	strcpy(tmpStr, currentPwd);
	char str[32];
	memset(str, 0, 32);
	FILE_FS * upperDirFsP = createFILE_FS(inodeP);
	fseekFs(upperDirFsP, 0);
	while(getContent_FS(upperDirFsP, str, 32)  == 32){
		strcat(currentPwd, str);
		FILE_FS * deleteFileFsP = openFile(currentPwd);
		if(deleteFileFsP == NULL){
			continue;
		}
		if(isDir(deleteFileFsP->inodeP)){
			if(strcmp(str, "..") == 0){
				free(deleteFileFsP->mem);
				free(deleteFileFsP);
				continue;
			}
			removeDir(delDirFsP->inodeP, str);
		}
		else{
			removeFile(delDirFsP->inodeP, str);
		}
		free(deleteFileFsP->mem);
		free(deleteFileFsP);
	}

	//从上层目录删除这个文件夹的记录
	removeRecordFromDir(upperDirFsP, dirName);

	freeFILE_FS(delDirFsP);
	free(upperDirFsP->mem);
	free(upperDirFsP);

	//恢复工作目录
	strcpy(currentPwd, pwdStore);
	return 0;

	setFreeInodeNumber(superBlockPointer, inodeP->inodeNumber);
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

	char pwdStore[256];
	strcpy(pwdStore, currentPwd);
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
		int length = strlen(currentPwd);
		if(currentPwd[length-1] != '/'){
			strcat(path, "/");
		}
		strcat(path, fileName);
		fileFsP->inodeP = getInode(path);
	}
	if(fileFsP->inodeP == NULL){
		strcpy(currentPwd, pwdStore);
		return NULL;
	}
	fileFsP->offset = 0;
	readFileContent(fileFsP);

	//复制内存不行，之间的数据关系还在，依然会被释放掉，导致访问错误
	//INODE * bakInodeP = copyINODE(fileFsP->inodeP);
	//fileFsP->inodeP = bakInodeP;
	strcpy(currentPwd, pwdStore);
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

int addBlockNumber(FILE_FS * fileFsP, unsigned int blockNumber)
{
	int * p = (int *)fileFsP->inodeP->mem;
	p[fileFsP->offset / superBlockPointer->blockSize + 1] = blockNumber;
	return 0;
}

void freeFILE_FS(FILE_FS * fileFsP)
{
	if(fileFsP->inodeP != superBlockPointer->inode){
		freeInode(fileFsP->inodeP);
	}
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