#pragma once

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
}User;
#endif

#include "INODE.h"

char ** parse(char * s);
void * Malloc(unsigned int Size_t);
char * ltrim(char * s, char c);
BOOL login();
BOOL _login(char * username, char * password);
int getInode(char * path); //等价于书上的NameI()
void writeBootSector();
int countMem(void * mem, int length);
int countChar(unsigned char ch);
int findZero(void * mem, int length);
BOOL hasCreateFileAuthority(INODE * inodeP, User * userP);
