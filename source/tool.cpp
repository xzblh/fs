#include <stdlib.h>
#include <string.h>

#include "tool.h"

extern User currentUser;

char ** parse(char * s)
{
	s = ltrim(s, ' ');
	char ** p = (char**)Malloc(sizeof(char *)* PARSE_TOKEN_LENGTH);
	char * _tmp = NULL;
	int i = 0;
	_tmp = strtok(s, " ");
	while (TRUE){
		if (_tmp == NULL){
			break;
		}
		else{
			_tmp = ltrim(_tmp, ' ');
			p[i] = _tmp;
		}
		_tmp = strtok(NULL, " ");
		i++;
	}
	return p;
}


void * Malloc(unsigned int Size_t)
{
	void * _p = malloc(Size_t);
	if (_p == NULL)[
		printf("内存申请失败！\r\n");
		exit(-3);
	]
	return _p;
}

char * ltrim(char * s, char c)
{
	if (NULL == s){
		return NULL;
	}
	char * p = s;
	while (*p == c){
		p++;
	}
	return p;
}


void login()
{
	char username[16];
	char password[16];
	printf("输入用户名：\r\n");
	while (scanf("%s", username) < 1)){
		printf("输入用户名：\r\n");
		continue;
	}
	printf("输入密码：\r\n"); //这个地方需要让屏幕不进行回显
	while (scanf("%s", username) < 1)){
		printf("输入密码：\r\n");
		continue;
	}
	_login(username, password);
}