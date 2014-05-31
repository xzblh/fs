#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"
#include "tool.h"
#include "superBlock.h"

extern SUPER_BLOCK * superBlockPointer;
extern User currentUser;
extern FILE * dataFp ;

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
	FILE_FS * fileFsP = openFile("/user");
	if(fileFsP == NULL){
		printf("无法打开用户文件，登陆失败，或者删除data.txt文件重新运行模拟程序！\r\n");
		return FALSE;
	}
	User * userP = getUser(fileFsP);
	if(strcmp(userP->username, username) == 0 && strcmp(userP->passwd, password) == 0){
		return TRUE;
	}
	else{
		return FALSE;
	}
}

INODE * getInode(char * path) //等价于书上的NameI()
{
	if(NULL == path){
		return NULL;
	}
	if(path[0] != '/'){
		return NULL;
	}
	if(strcmp(path, "/") == 0){
		return superBlockPointer->inode;
	}
	return NULL;
}

void writeBootSector()
{
	char spaces[] = "boot sector ";
	int i = 0;
	while(i<(512/sizeof(spaces))){
		fwrite(spaces,sizeof(spaces), 1,dataFp);
		i++;
	}
	i = 0;
	while(i< (512%sizeof(spaces))){
		fwrite(" ",1, 1,dataFp);
		i++;
	}
}

int countMem(void * mem, int length)
{
	char * str = (char*)mem;
	if(NULL == mem){
		return 0;
	}
	int count = 0;
	unsigned char ch;
	int i = 0;
	while(i < length){
		ch = str[i];
		count += countChar(ch);
		i++;
	}
	return count;
}

int countChar(unsigned char ch)
{
	int count = 0;
	while(ch){
		ch = ch >> 1;
		count ++;
	}
	return count;
}

int findZero(void * mem, int length) //查找并设置bitmap
{
	char * str = (char*)mem;
	if(NULL == mem){
		return 0;
	}
	int pos = 0;
	unsigned char ch;
	int i = 0;
	while(i < length){
		ch = str[i];
		if(ch == (unsigned char)255){
			pos += 8;
		}
		else{
			int tmp = countChar(ch);
			unsigned char a = 1 << tmp;
			str[i] = ch | a;
			return pos + tmp;
		}
		i++;
	}
	return -1;
}


BOOL hasCreateFileAuthority(INODE * inodeP, User * userP)
{
	if(userP->UID == 0){
		//ROOT 则返回TRUE
		return TRUE;
	}
	if(userP->UID == inodeP->UID){
		return TRUE;
	}
	if(userP->GID == inodeP->GID){
		if(inodeP->authority && _GROUP_WRITE_DEFINE_)
			return TRUE;
		else
			return FALSE;
	}
	if(inodeP->authority && _OTHER_WRITE_DEFINE_)
		return TRUE;
	else
		return FALSE;
}

void writeNull(int count, FILE * fp)
{
	char c = 0;
	int i = 0;
	while(i < count){
		fwrite(&c, 1, 1, fp);
	}
}

void writeChar(char * s, int count, FILE * fp) //把某个字符写到文件当前位置count次
{
	int i = 0;
	while(i < count){
		fwrite(s, strlen(s), 1, fp);
	}
}