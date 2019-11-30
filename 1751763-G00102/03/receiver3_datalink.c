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
    kill(getpid(),MYSIG_CONTINUE);
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
    kill(getpid(),MYSIG_CONTINUE);
}
static void wait_for_event(event_type *event) //�����������ȴ��¼�����
{
    signal(MYSIG_FRAME_ARRIVAL, SIGHANDLER_MYSIG_FRAME_ARRIVAL);
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
    const char *network_proc = "network";

    if (get_first_pid(network_proc)==-1)//��ΪҪ������㷢�ź����������ļ�������Ҫ�ȴ������
    {
        printf("plz start netwrok_layer first");
        return 0;
    }
    printf("datalink ready \n");

    seq_nr frame_expected;
    frame r, s;
    event_type event;
    
#ifndef MYDEBUG
    frame_expected = 0;

    signal(MYSIG_FRAME_ARRIVAL, SIGHANDLER_MYSIG_FRAME_ARRIVAL);
    signal(MYSIG_CHSUM_ERR, SIGHANDLER_MYSIG_CHSUM_ERR);

    while (true)
    {
        wait_for_event(&event); //�������¼�
        if (event == frame_arrival)
        {
            from_physical_layer(&r);
            if (r.seq == frame_expected)
            { //���ƥ��
                to_network_layer(&r.info);
                enable_network_layer_read(network_proc);
                inc(frame_expected);
            }
            else
            {
                record_err(frame_expected,rec_mismatch_data);
                record_repeat(-1,0,1-frame_expected,rec_mismatch_data);
            }
            
            s.ack = 1 - frame_expected; //�����Ƿ�ƥ��,0-1�任
            s.kind=ack;
            s.seq=0xffffffff;
            to_physical_layer(&s);
        }
        else
            record_err(frame_expected,cksum_err);
        
        //event = cksum_err��������ֱ�ӵȴ���һ���¼�
    }
#endif

#ifdef MYDEBUG

#endif
}