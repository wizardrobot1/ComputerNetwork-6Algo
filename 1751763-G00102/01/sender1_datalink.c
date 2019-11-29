#include "../common/common.h"
#include "../common/tools.h"
#include "../common/d2n_layer.h"

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
    

    const char *network_proc = "network";
    signal(MYSIG_NETWORK_LAYER_READY, SIG_IGN); //屏蔽MYSIG_NETWORK_LAYER_READY信号
    printf("datalink ready \n");

    frame s;
    packet buffer;

    
#ifndef MYDEBUG

    while (1)
    {
        from_network_layer(&buffer);
        enable_network_layer(network_proc);//通知网络层发下一数据
        memcpy(s.info.data, buffer.data, MAX_PKT);
        s.kind=data;
        to_physical_layer(&s);
    }
#endif

#ifdef MYDEBUG
    

    int ftest = open("test1", O_WRONLY | O_CREAT, 0644);
    while (1)
    {
        from_network_layer(&buffer);
        enable_network_layer(network_proc);//通知网络层发下一数据
        memcpy(s.info.data, buffer.data, 1024);
        write(ftest, s.info.data, MAX_PKT);

        //to_physical_layer(&s);
    }

    close(ftest);
#endif
}