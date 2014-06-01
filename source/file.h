#ifndef _FILE_HEADER_
#define _FILE_HEADER_

	#include "INODE.h"

	typedef struct _file{
		INODE * inodeP; //文件的INODE对象
		unsigned int offset;//文件指针的当前偏移量
		void * mem; //间接block的内容
	}FILE_FS;

	int getc_FS(FILE_FS * fileFsP);
	unsigned int getContent_FS(FILE_FS * fileFsP, void * mem, unsigned int length);

#endif


int writeContent(INODE * inodeP, void * mem, int length, int offset);
				//  确定文件INODE编号   要写的内存区  要写的内存长度    将要写的文件偏移量
int writeFileContent(FILE_FS * fileFsP);
int readFileContent(FILE_FS * fileFsP);
void writeFileBuffer(FILE_FS * fileFsP, char * s);
BOOL updateFileBuffer(FILE_FS * fileFsP, void * mem, unsigned int length, unsigned int pos);
int writeFileBuffer(FILE_FS * fileFsP, void * mem, int length);
//void readFileBuffer(FILE_FS * fileFsP); //这个函数好像同readFileContent()功能差不多
int createFile(INODE * inodeP, char * fileName);
int removeFile(INODE * inodeP);
int removeFile(INODE * inodeP, char * fileName);
int createDir(INODE * inodeP, char * dirName);
int removeRecordFromDir(FILE_FS * dirFsP, char * fileName);
int removeDir(INODE * inodeP, char * dirName);
void writeAddUser(User * userP, FILE_FS * fileFsP);
User * getUser(FILE_FS * fileFsP);
FILE_FS * openFile(char * fileName);
FILE_FS * createFILE_FS(INODE* inodeP);
void * getLine(FILE_FS * fileFsP, void * mem);
int getCurrentBlockNumber(FILE_FS * fileFsP);
void freeFILE_FS(FILE_FS * fileFsP);
int fseekFs(FILE_FS * fileFsP, unsigned int offset);
int getFileInodeInFolder(FILE_FS * fileFsP, char * fileName);