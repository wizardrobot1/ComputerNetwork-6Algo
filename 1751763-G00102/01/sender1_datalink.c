#include"../common/common.h"
#include"../common/tools.h"

int main()
{
    frame s;
    packet buffer;
    
    int pids[100];
    const char *network_proc = "sender1_network";
    while (getpid_by_name(network_proc, pids) == 0) //理论上和实际上返回值都应该是1
    {
        sleep(1);//等待网络层打开
    }
    printf("sender_datalink ready\n");

    //int ftest=open("test1",O_WRONLY | O_CREAT, 0644);
    while(1)
    {
        from_network_layer(&buffer,pids[0]);
        memcpy(s.info.data,buffer.data,1024);
        //write(ftest,s.info.data,MAX_PKT);
        
        //to_physical_layer(&s.info);
    }

    //close(ftest);
}