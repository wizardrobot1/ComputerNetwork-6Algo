#include "../common/common.h"
#include "../common/tools.h"
#include "../common/d2n_layer.h"
#include "../common/savelog.h"
#define MAX_SEQ 1
#define inc(k) if(k<MAX_SEQ) k=k+1; else k=0;
//#define MYDEBUG

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
}

static void SIGHANDLER_MYSIG_TIMEOUT(int signo)
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
    p->event = timeout;
    p->next = NULL;
}

static void SIGHANDLER_MYSIG_CHSUM_ERR(int signo)
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
    p->event = cksum_err;
    p->next = NULL;
}

static void wait_for_event(event_type *event) //阻塞函数，等待事件发生
{
    signal(MYSIG_FRAME_ARRIVAL, SIGHANDLER_MYSIG_FRAME_ARRIVAL);
    signal(MYSIG_TIMEOUT, SIGHANDLER_MYSIG_TIMEOUT);
    signal(MYSIG_CHSUM_ERR, SIGHANDLER_MYSIG_CHSUM_ERR);
    if(!eventqueue)
        pause();

    event_queue p=eventqueue;    
    *event = eventqueue->event;
    eventqueue=eventqueue->next;
    free(p);
}

int main()
{

    signal(MYSIG_NETWORK_LAYER_READY, SIG_IGN); //屏蔽MYSIG_NETWORK_LAYER_READY信号
    const char *network_proc = "network";
    const char *physical_proc= "physical";
    if (get_first_pid(network_proc)==-1)//因为要未经询问不阻塞取3层数据，所以要先打开3层放数据
    {
        printf("plz start netwrok_layer first");
        return 0;
    }
    if (get_first_pid(physical_proc)==-1)//因为要向1层发信号提醒它读文件，所以要先打开1层（2层发文件给1层如果信号没收到他就不会收）
    {
        printf("plz start physical_layer first");
        return 0;
    }
    printf("datalink ready \n");

    seq_nr next_frame_to_send;
    frame s;
    packet buffer;
    event_type event;

#ifndef MYDEBUG
    next_frame_to_send = 0;      //初始帧号为0
    from_network_layer(&buffer); //取首帧
    enable_network_layer(network_proc);//通知网络层发下一数据
    signal(MYSIG_FRAME_ARRIVAL, SIGHANDLER_MYSIG_FRAME_ARRIVAL);
    signal(MYSIG_TIMEOUT, SIGHANDLER_MYSIG_TIMEOUT);
    signal(MYSIG_CHSUM_ERR, SIGHANDLER_MYSIG_CHSUM_ERR);
    while (1)
    {
        s.info = buffer;
        s.seq = next_frame_to_send;
        s.kind=data;
        to_physical_layer(&s);
        start_timer(s.seq);
        wait_for_event(&event); //等三个事件
        if (event == frame_arrival)
        {
            from_physical_layer(&s);
            if (s.ack == next_frame_to_send)
            {
                stop_timer(s.ack);
                from_network_layer(&buffer);
                enable_network_layer(network_proc);//通知网络层发下一数据
                inc(next_frame_to_send);
            }
            else//ACK到，但s.ack不对
            {
                record_err(next_frame_to_send,rec_mismatch_ack);
                record_repeat(next_frame_to_send,1,-1,rec_mismatch_ack);
            }                    
        }     
        else if (event==cksum_err)//如果event=cksum_err
        {
            record_err(next_frame_to_send,rec_cksum_err);
            record_repeat(next_frame_to_send,1,-1,rec_cksum_err);
        }
        else if (event==timeout)//如果event=timout
        {
            record_err(next_frame_to_send,rec_timeout);
            record_repeat(next_frame_to_send,1,-1,rec_timeout);
        }
        
    } //end of while

#endif

#ifdef MYDEBUG

#endif
}