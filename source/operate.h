#ifndef _OPERATE_HEADER_
#define _OPERATE_HEADER_
#endif 

#include "tool.h"
#include "INODE.h"

void CD(char ** cmds); //get into foler  大致完成
void TOUCH(char ** cmds); //create file  大致完成
void RM(char ** cmds); //remove file
void MKDIR(char ** cmds); //create folder 大致完成
void RMDIR(char ** cmds); //remove folder 
void LS(char ** cmds); //list files attributes 大致完成
void WRITE(char ** cmds); //write file  大致完成
void READ(char ** cmds); //read file  大致完成
void UMASK(char ** cmds); //set the file or folder's attributes