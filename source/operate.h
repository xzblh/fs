#ifndef _OPERATE_HEADER_
#define _OPERATE_HEADER_
#endif 

#include "tool.h"
#include "INODE.h"

void CD(char ** cmds); //get into foler
void TOUCH(char ** cmds); //create file
void RM(char ** cmds); //remove file
void MKDIR(char ** cmds); //create folder
void RMDIR(char ** cmds); //remove folder
void LS(char ** cmds); //list files attributes
void WRITE(char ** cmds); //write file
void READ(char ** cmds); //read file
void UMASK(char ** cmds); //set the file or folder's attributes