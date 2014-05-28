#include <stdio.h>
#include <string.h>

#include "tool.h"

#define PWD_LENGTH 1024

char userFileName[] = "/user";
char groupFileName[] = "/group";

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
		}
		else if (strcmp(cmds[0], "touch") == 0){
			//创建文件
		}
		else if (strcmp(cmds[0], "rm") == 0){
			//删除文件
		}
		else if (strcmp(cmds[0], "mkdir") == 0){
			//创建文件夹
		}
		else if (strcmp(cmds[0], "rmdir") == 0){

		}
		else if (strcmp(cmds[0], "ls") == 0){
			//罗列当前目录下文件属性
		}
		else if (strcmp(cmds[0], "write") == 0){
			//写文件操作
		}
		else if (strcmp(cmds[0], "read") == 0){
			//读文件内容

		}
		else if (strcmp(cmds[0], "umask") == 0){
			//文件或目录权限设置
		}
		else{
			continue;
		}
	}
}

void main()
{
	//进行登陆检查

	if(login())
	{
		run();
	}
	else{
		printf("登陆失败！\r\n");
	}

}
