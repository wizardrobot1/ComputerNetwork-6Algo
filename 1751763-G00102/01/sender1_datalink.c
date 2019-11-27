#include "../common/common.h"
#include "../common/tools.h"

//#define MYDEBUG
/*
int getpid_by_name2(const char* proc_name,int *pids)
{
        char str_part1[100]="ps -ef | grep \'";
        char *str_part2="\' | awk \'{print $8}\'";
        char ans[4][100];
        strcat(str_part1,proc_name);
        strcat(str_part1,str_part2);
        
        int count=0;
        FILE *fp = popen(str_part1,"r");
        while (NULL != fgets(ans[count], 100, fp)) 
        {   
                ++count;
        }   
        if (count==3)
        {
            printf("%s\n",ans[0]);
            printf("%s\n",ans[1]);
            printf("%s\n",ans[2]);
        }
        pclose(fp);
        return count;
}
*/

int main()
{
    

    int pids[100];
    const char *network_proc = "sender1_network";
    signal(MYSIG_NETWORK_LAYER_READY, SIG_IGN); //屏蔽MYSIG_NETWORK_LAYER_READY信号

    frame s;
    packet buffer;
    
#ifndef MYDEBUG

    while (getpid_by_name(network_proc, pids) != 3) //一个sh , 一个 grep , 一个 ./sender1_network
    {
        sleep(1); //等待网络层打开
    }

    while (1)
    {
        from_network_layer(&buffer, pids[0]);
        memcpy(s.info.data, buffer.data, 1024);
        s.kind=data;
        //to_physical_layer(&s);
    }
#endif

#ifdef MYDEBUG
    while (getpid_by_name(network_proc, pids) != 3) //一个sh , 一个 grep , 一个 ./sender1_network
    {
        sleep(1); //等待网络层打开
    }
    printf("receiver_datalink ready %d\n", pids[0]);

    int ftest = open("test1", O_WRONLY | O_CREAT, 0644);
    while (1)
    {
        from_network_layer(&buffer, pids[0]);
        memcpy(s.info.data, buffer.data, 1024);
        write(ftest, s.info.data, MAX_PKT);

        //to_physical_layer(&s);
    }

    close(ftest);
#endif
}