#pragma once

#include <stdio.h>
#ifndef _TOOL_HEADER_

#define _TOOL_HEADER_

#define BOOL int
#define TRUE 1
#define FALSE 0

#define PARSE_TOKEN_LENGTH 5

typedef struct _user{
	char * username;
	char * passwd;
	int UID;
	int GID;
	unsigned int umask;
}User;
#endif

#include "INODE.h"

char ** parse(char * s);
void * Malloc(unsigned int Size_t);
int Fwrite(void * mem, size_t size, size_t count, FILE * fp);
char * ltrim(char * s, char c);
BOOL login();
BOOL _login(char * username, char * password);
INODE * getInode(char * path); //等价于书上的NameI()
void writeBootSector();
int countMem(void * mem, int length);
int countChar(unsigned char ch);
int findZero(void * mem, int length);
int setZero(void * mem, unsigned int length, unsigned int pos);
BOOL hasCreateFileAuthority(INODE * inodeP, User * userP);
void writeNull(unsigned int count, FILE * fp);
void writeChar(char * s, int count, FILE * fp);  //把某个字符写到文件当前位置count次
void * pathCat(char * path, char * path2);
void setUmask(User * userP, char * );
BOOL isDigit(char * s); //判断一个字符串是否全是数字
