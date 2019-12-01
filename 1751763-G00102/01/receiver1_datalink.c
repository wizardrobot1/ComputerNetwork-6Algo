#include "../common/common.h"
#include "../common/tools.h"
#include "../common/d2n_layer.h"
#include "../common/p2d_layer.h"

static event_queue eventqueue= NULL;

static void SIGHANDLER_MYSIG_FRAME_ARRIVAL(int signo)
{
    event_queue p=eventqueue;
    while(p&&p->next)
    {
        p=p->next;
    }
    if(!p)
    {
        eventqueue=(event_queue)malloc(sizeof(struct event_queue_node));
        p=eventqueue;
    }
    else
    {
        p->next=(event_queue)malloc(sizeof(struct event_queue_node));
        p=p->next;
    }
    p->event = frame_arrival;
    p->next = NULL;
    //kill(getpid(),MYSIG_CONTINUE);
}


static void wait_for_event(event_type *event) //阻塞函数，等待事件发生
{
    signal(MYSIG_FRAME_ARRIVAL, SIGHANDLER_MYSIG_FRAME_ARRIVAL);
    if(!eventqueue)
        pause();

    event_queue p=eventqueue;    
    *event = eventqueue->event;
    eventqueue=eventqueue->next;
    free(p);
}
//#define MYDEBUG
int main()
{
#ifndef MYDEBUG
    const char *network_proc = "network";

    if (get_first_pid(network_proc)==-1)//因为要向网络层发信号提醒它读文件，所以要先打开网络层
    {
        printf("plz start network_layer first");
        return 0;
    }

    printf("datalink ready \n");

    frame r;
    event_type event;    

	int mark=0;
    while (1)
    {
        wait_for_event(&event);
        from_physical_layer(&r);
    
        to_network_layer(&r.info);
        enable_network_layer_read(network_proc);
    }
#endif
    
#ifdef MYDEBUG
    printf("DEBUGING\n");
    const char *network_proc = "network";

    if (get_first_pid(network_proc)==-1)//因为要向网络层发信号提醒它读文件，所以要先打开网络层
    {
        printf("plz start network_layer first");
        return 0;
    }
    frame r;
    event_type event; 
    char share_file_name[MAX_FILENANE_LEN];
    char temp[12];
    int share_file, seq_PKT = 0;
    while (1)
    {
        sprintf(share_file_name, "%s%04d", PHYSICAL_DATALINK_SHARE_FILE, seq_PKT);//用发送方网络层发给链路层的PKT来测试
        inc_seq_PKT(seq_PKT);
        share_file = open(share_file_name, O_RDONLY);
        if (share_file==-1)
            break;
        read(share_file,temp,12);
        read(share_file, r.info.data, MAX_PKT);
        to_network_layer(&r.info);
        enable_network_layer_read(network_proc);
        close(share_file);
    }
#endif
// 	disable_network_layer(NETWORK);
}                                