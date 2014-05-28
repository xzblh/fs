#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tool.h"
#include "superBlock.h"
#include "operate.h"

#define PWD_LENGTH 1024

char userFileName[] = "/user";
char groupFileName[] = "/group";
char dataFileName[] = "data.txt";

FILE * dataFp = NULL;
SUPER_BLOCK superBlockPointer = NULL;

char currentPwd[PWD_LENGTH];
User currentUser;

void run()
{
	char command[16];
	char ** cmds = NULL;
	int n = 0;

	while (TRUE)
	{
		n = scanf("%[^\n]", command);
		if (n < 1){
			continue;
		}
		cmds = parse(command);
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
void inditSuperBlock()
{

}
int main()
{
	//进行登陆检查

	dataFp = fopen(dataFileName, "r+");

	if(NULL == dataFp){
		printf("Can not open emulate file: %s.\r\n", dataFileName);
		exit(-1);
	}
	else{
		//初始化超级块数据结构
		inditSuperBlock();
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
