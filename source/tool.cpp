#include <stdio.h>
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
	if (_p == NULL){
		printf("内存申请失败！\r\n");
		exit(-3);
	}
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


BOOL login()
{
	char username[16];
	char password[16];
	printf("input Username:\r\n");
	while (scanf("%s", username) < 1){
		printf("input Username;\r\n");
		continue;
	}
	fflush(stdin);
	printf("input Password;\r\n"); //这个地方需要让屏幕不进行回显
	while (scanf("%s", password) < 1){
		printf("input Password:\r\n");
		continue;
	}
	return _login(username, password);
}

BOOL _login(char * username, char * password)
{
	return TRUE;
}