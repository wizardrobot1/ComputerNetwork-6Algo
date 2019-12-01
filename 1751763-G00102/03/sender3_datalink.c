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

static void wait_for_event(event_type *event) //�����������ȴ��¼�����
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

    signal(MYSIG_NETWORK_LAYER_READY, SIG_IGN); //����MYSIG_NETWORK_LAYER_READY�ź�
    const char *network_proc = "network";
    const char *physical_proc= "physical";
    if (get_first_pid(network_proc)==-1)//��ΪҪδ��ѯ�ʲ�����ȡ3�����ݣ�����Ҫ�ȴ�3�������
    {
        printf("plz start netwrok_layer first");
        return 0;
    }
    if (get_first_pid(physical_proc)==-1)//��ΪҪ��1�㷢�ź����������ļ�������Ҫ�ȴ�1�㣨2�㷢�ļ���1������ź�û�յ����Ͳ����գ�
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
    next_frame_to_send = 0;      //��ʼ֡��Ϊ0
    from_network_layer(&buffer); //ȡ��֡
    enable_network_layer(network_proc);//֪ͨ����㷢��һ����
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
        wait_for_event(&event); //�������¼�
        if (event == frame_arrival)
        {
            from_physical_layer(&s);
            if (s.ack == next_frame_to_send)
            {
                stop_timer(s.ack);
                from_network_layer(&buffer);
                enable_network_layer(network_proc);//֪ͨ����㷢��һ����
                inc(next_frame_to_send);
            }
            else//ACK������s.ack����
            {
                record_err(next_frame_to_send,rec_mismatch_ack);
                record_repeat(next_frame_to_send,1,-1,rec_mismatch_ack);
            }                    
        }     
        else if (event==cksum_err)//���event=cksum_err
        {
            record_err(next_frame_to_send,rec_cksum_err);
            record_repeat(next_frame_to_send,1,-1,rec_cksum_err);
        }
        else if (event==timeout)//���event=timout
        {
            record_err(next_frame_to_send,rec_timeout);
            record_repeat(next_frame_to_send,1,-1,rec_timeout);
        }
        
    } //end of while

#endif

#ifdef MYDEBUG

#endif
}