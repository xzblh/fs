#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "file.h"
#include "tool.h"
#include "superBlock.h"
#include "operate.h"

#define PWD_LENGTH 1024

char userFileName[] = "/user";
char groupFileName[] = "/group";
char dataFileName[] = "data.txt";

FILE * dataFp = NULL;
SUPER_BLOCK * superBlockPointer = NULL;

char currentPwd[PWD_LENGTH];
User * currentUser = NULL;

void run()
{
	char command[16];
	char ** cmds = NULL;
	int n = 0;

	while (TRUE)
	{
		n = scanf("%15[^\n]", command);
		getchar();
		if (n < 1){
			continue;
		}
		cmds = parse(command);
		if(cmds == NULL){
			printf("params number is not right!\r\n");
			continue;
		}
		if (strcmp(cmds[0], "cd") == 0){
			//进入目录
			CD(cmds);
		}
		else if (strcmp(cmds[0], "touch") == 0){
			//创建文件
			TOUCH(cmds);
		}
		else if (strcmp(cmds[0], "rm") == 0){
			//删除文件
			RM(cmds);
		}
		else if (strcmp(cmds[0], "mkdir") == 0){
			//创建文件夹
			MKDIR(cmds);
		}
		else if (strcmp(cmds[0], "rmdir") == 0){
			//删除文件夹
			RMDIR(cmds);
		}
		else if (strcmp(cmds[0], "ls") == 0){
			//罗列当前目录下文件属性
			LS(cmds);
		}
		else if (strcmp(cmds[0], "write") == 0){
			//写文件操作
			WRITE(cmds);
		}
		else if (strcmp(cmds[0], "read") == 0){
			//读文件内容
			READ(cmds);
		}
		else if (strcmp(cmds[0], "umask") == 0){
			//文件或目录权限设置
			UMASK(cmds);
		}
		else{
			continue;
		}
	}
}
void initSuperBlock()
{
	superBlockPointer = (SUPER_BLOCK *)Malloc(sizeof(SUPER_BLOCK));
	superBlockPointer->blockCount = 8*512*128; // Inode数目*每个Inode能够访问的数目（即512/4=128）  文件最大大小约为256M
	// 大概相当于这么多，没有去掉超级块、bitmap和inode节点占用的数据块
	superBlockPointer->inodeCount = 8*512;
	//superBlockPointer->blockFreeCount = ;
	//superBlockPointer->inodeFreeCount = ;

	superBlockPointer->blockSize = 512;
	superBlockPointer->inodeSize = 64;
	superBlockPointer->blockBitMapCount = 8;
	superBlockPointer->blockBitMapStart = 2;
	superBlockPointer->inodeBitMapCount = 8;
	superBlockPointer->inodeBitMapStart = superBlockPointer->blockBitMapStart + superBlockPointer->blockBitMapCount;
	//从上，可以看出data.txt文件至少包含一个启动块，一个超级块，一个blockBitMap，即8块，一个InodeBitMap， 即8块。
	//一个Inode节点区，该区大概8*512*64字节，即512个扇区。总共 1+1+8+8+512 = 530

	//printf("%d\r\n", 
	//	(1+1+superBlockPointer->blockBitMapCount+superBlockPointer->inodeBitMapCount + 
	//	superBlockPointer->inodeCount*superBlockPointer->inodeSize/superBlockPointer->blockSize));
	//getchar();
	writeBootSector();
	superBlockPointer->bBitMap = Malloc(getBlockBitMapByteCount(superBlockPointer));
	superBlockPointer->iBitMap = Malloc(getInodeBitMapByteCount(superBlockPointer));
	fseek(dataFp, 0, SEEK_END);
	int l = ftell(dataFp);
	if(ftell(dataFp) < getInodeAreaOffset(superBlockPointer) ){
			//初始化文件系统 如创建根文件夹，初始化用户等

			currentUser->GID = 0;
			currentUser->UID = 0;
			strcpy(currentUser->passwd, "root");
			strcpy(currentUser->username, "root");
			superBlockPointer->blockFreeCount = superBlockPointer->blockCount - 530;
			superBlockPointer->inodeFreeCount = superBlockPointer->inodeCount;
			//写起始扇区
			writeSuperBlock(superBlockPointer, dataFp);

			//初始化超级块的数据
			memset(superBlockPointer->bBitMap, 0xFF, 66);
			memset((void*)((char *)(superBlockPointer->bBitMap)+66), 0x03, 1);
			memset((void*)((char *)(superBlockPointer->bBitMap)+67), 0, getBlockBitMapByteCount(superBlockPointer) -67);
			memset(superBlockPointer->iBitMap, 0, getInodeBitMapByteCount(superBlockPointer));

			//写bitmap结构
			writeBitMap(superBlockPointer, dataFp);

			//创建根节点，即“/”文件夹
			writeRoot(superBlockPointer);
			
			createFile(superBlockPointer->inode, "user");
			FILE_FS * fileFsP = openFile("/user");
			if(fileFsP == NULL){
				printf("can not open file: /user\r\n");
				exit(0);
			}
			writeAddUser(currentUser, fileFsP);
			freeFILE_FS(fileFsP);
	}
	else{
		readBitMap(superBlockPointer, dataFp);
		readRoot(superBlockPointer);
	}
}

int main()
{
	//进行登陆检查
	
	dataFp = fopen(dataFileName, "rb+");
	if(NULL == dataFp){
		dataFp = fopen(dataFileName, "wb+");
	}

	currentUser = (User*)Malloc(sizeof(User));
	currentUser->username = (char*)Malloc(16);
	currentUser->passwd = (char*)Malloc(16);
	if(NULL == dataFp){
		printf("Can not open emulate file: %s.\r\n", dataFileName);
		getchar();
		exit(-1);
	}
	else{
		//初始化超级块数据结构
		//及超级块的数据校验
		initSuperBlock();
	}

	if(login())
	{
		run();
	}
	else{
		printf("登陆失败！\r\n");
	}
	return 0;
}
