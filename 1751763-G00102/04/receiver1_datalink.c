#include "../common/common.h"
#include "../common/tools.h"

//#define MYDEBUG
static event_type catch_event;
static void SIGHANDLER_MYSIG_FRAME_ARRIVAL(int signo)
{
    catch_event = frame_arrival;
}
static void wait_for_event(event_type *event) //阻塞函数，等待事件发生
{
    signal(MYSIG_FRAME_ARRIVAL, SIGHANDLER_MYSIG_FRAME_ARRIVAL);
    pause();
    *event = catch_event;
}


int main()
{
    

    const char *network_proc = "network";

    if (get_first_pid(network_proc)==-1)//因为要向网络层发信号提醒它读文件，所以要先打开网络层
    {
        printf("plz start netwrok_layer first");
        return 0;
    }    
    printf("datalink ready \n");

    frame r;
    event_type event;
#ifndef MYDEBUG
    while (1)
    {
        wait_for_event(&event);
        from_physical_layer(&r);
        to_network_layer(&r.info);
        enable_network_layer_read(network_proc);
    }
#endif

#ifdef MYDEBUG
    char share_file_name[MAX_FILENANE_LEN];
    int share_file, seq_PKT = 0;
    while (1)
    {
        sprintf(share_file_name, "%s%04d", NETWORK_DATALINK_SAHRE_FILE, seq_PKT);//用发送方网络层发给链路层的PKT来测试
        inc_seq_PKT(seq_PKT);
        share_file = open(share_file_name, O_RDONLY);
        if (share_file==-1)
            break;
        read(share_file, r.info.data, MAX_PKT);
        to_network_layer(&r.info);
        enable_network_layer_read(network_proc);
        close(share_file);
    }
#endif
}