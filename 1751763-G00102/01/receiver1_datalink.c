#include"../common/common.h"
#include"../common/tools.h"


static event_type catch_event;
static void SIGHANDLER_MYSIG_FRAME_ARRIVAL(int signo)
{
    catch_event=frame_arrival;
}
static void wait_for_event(event_type *event)//阻塞函数，等待事件发生
{
    signal(MYSIG_FRAME_ARRIVAL, SIGHANDLER_MYSIG_FRAME_ARRIVAL);
    pause();
    *event=catch_event;
}

int main()
{
    frame r;
    event_type event;

    int pids[100];
    const char *network_proc = "receiver1_network";
    while (getpid_by_name(network_proc, pids) != 3) 
    {
        sleep(1);//等待网络层打开
    }
    printf("receiver_datalink ready %d\n",pids[0]);

    char share_file_name[256],buffer[1024];
    int share_file,seq_PKT=0;
    while(1)
    {
        wait_for_event(&event);
        //from_physical_layer(&r);
        to_network_layer(&r.info,pids[0]);
    }
}