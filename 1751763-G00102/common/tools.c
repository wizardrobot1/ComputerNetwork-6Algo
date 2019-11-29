#include "tools.h"

void error_exit(char *name)
{
   perror(name);
   exit(-1);
}

int getpid_by_name(const char* proc_name,int *pids)
{
        char str_part1[100]="ps -ef | grep \'";
        char *str_part2="\' | awk \'{print $2}\'";
        char ans[10]="";
        strcat(str_part1,proc_name);
        strcat(str_part1,str_part2);
        
        int count=0;
        FILE *fp = popen(str_part1,"r");
        while (NULL != fgets(ans, 10, fp)) 
        {   
                *pids++=atoi(ans);
                ++count;
        }   
        pclose(fp);
        return count;
}

int set_lock(int fd,int type)  
{  
    struct flock old_lock,lock;  
    lock.l_whence = SEEK_SET;  
    lock.l_start = 0;  
    lock.l_len = 0;  
    lock.l_type = type;  
    lock.l_pid = -1;  
 
      
    if ((fcntl(fd,F_SETLKW,&lock)) < 0)  
    {  
        error_exit("Lock failed");  
        return 1;  
    }  
    
    return 0;  
}  


int getPid(const char*str)			// be sure that str is correct 
{
	char buff[10] = {0};int pid=0;
	char comd[100]={0};	
	sprintf(comd,"ps -e | grep \'%s\' | awk \'{print $1}\'",str);

	FILE *fp=NULL;
	while(!pid){
		fp = popen(comd, "r");    
		while (NULL != fgets(buff, 10, fp));
    	pclose(fp);

    	pid=atoi(buff);
	}

//    printf("pid:%d\n",pid);
    return pid;	
}
void sendSIG(int pid,int SIG)
{
	int ret;

	if((ret=kill(pid,SIG))!=0){
		printf("kill %d error\n",SIG);
		return ;
	}
	else
		printf("kill %d successfully\n",SIG);
}


int get_first_pid(const char* proc_name)//if success , return pid ; if failed , return -1
{
        char str_part1[100]="ps -ef | grep \'";
        char *str_part2="\' | awk \'{print $2}\'";
        char ans[10]="";
        int pids[10];
        strcat(str_part1,proc_name);
        strcat(str_part1,str_part2);
        
        int count=0;
        FILE *fp = popen(str_part1,"r");
        while (NULL != fgets(ans, 10, fp)) 
        {   
                pids[count]=atoi(ans);
                ++count;
                if (count==3)//grep,sh,./proc_name 如果有三行，那么第一行就是要找的进程，存在pid[0]里
                    break;
        }   
        pclose(fp);

        if (count==3)
            return pids[0];
        else 
            return -1;
}

int get_datatime(char * psDateTime)
{
    long nSeconds;
    struct tm * pTM;
    
    time(&nSeconds);
    pTM = localtime(&nSeconds);

    /* 系统日期和时间,格式: yyyymmddHHMMSS */
    sprintf(psDateTime, "%04d-%02d-%02d %02d:%02d:%02d",
            pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday,
            pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
            
    return 0;
}