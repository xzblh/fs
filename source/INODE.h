#include <time.h>

#include "tool.h"

#ifndef _INODE_HEADER_
#define _INODE_HEADER_

	/*
	* 按照下面的结构计算，每个inode大小为4*9 = 36字节
	*/
	typedef struct _inode{
		time_t aTime; //通通转成时间戳  数字存储
		time_t cTime;
		time_t mTime;
		int GID;
		int UID;
		unsigned int authority;
		unsigned int  inodeNumber; //INODE节点的编号
		unsigned int  blockNumber; // 扇区块编号，一重间接方式使用 即单文件大小限制：512*512/4 = 64K
		unsigned int  length; // 文件长度
	}INODE;

#define DIR_DEFINE          0x00001000 //
#define FILE_DEFINE         0x00000800 //
#define ROOT_DEFINE         0x00000400
#define TMP_DEFINE          0x00000200

//用户权限宏
#define USER_READ_DEFINE    0x00000100
#define USER_WRITE_DEFINE   0x00000080
#define USER_EXEC_DEFINE    0x00000040

//用户组权限宏
#define GROUP_READ_DEFINE   0x00000020
#define GROUP_WRITE_DEFINE  0x00000010
#define GROUP_EXEC_DEFINE   0x00000008

//其他人的权限宏
#define OTHER_READ_DEFINE   0x00000004
#define OTHER_WRITE_DEFINE  0x00000002
#define OTHER_EXEC_DEFINE   0x00000001

#define _755_AUTHORITY_FILE_ 0x000009ED  //755 的 -rwxr-xr-x权限
#define _755_AUTHORITY_DIR_  0x000011ED  //755 的 drwxr-xr-x权限

#endif

INODE * createINODE();
void writeINODE(INODE * inodeP);
void writeAddUser(User * userP, INODE * inodeP);
void writeUser(User * userP, int offset);
void writeUser(User * userP);
BOOL isDir(INODE * inodeP);
