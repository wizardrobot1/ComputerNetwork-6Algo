#ifndef __TOOLS__H
#define __TOOLS__H

#include "common.h"
#endif

void error_exit(char *name);

int getpid_by_name(const char* proc_name,int *pids);

//LOCK TYPES:F_RDLCK , F_WRLCK  ,  F_UNLCK
int set_lock(int fd,int type);

void sendSIG(int pid,int SIG);

int getPid(const char*str);

int get_first_pid(const char* proc_name);//if success , return pid ; if failed , return -1