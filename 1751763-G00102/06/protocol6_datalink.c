#include "../common/common.h"
#include "../common/tools.h"
#include "../common/d2n_layer.h"
#include "../common/savelog.h"
#define MAX_SEQ 7 //保持2^n -1
#define inc(k)           \
    if (k < MAX_SEQ + 1) \
        k = k + 1;       \
    else                 \
        k = 0; //k=0~7 窗口的大小是MAX_SEQ+1 , 帧序号是0~MAX_SEQ
#define NR_BUFS ((MAX_SEQ + 1) / 2)//既是可同时发送帧的数量，也是接收方的缓存区大小
boolean no_nak = true;//记录对于当前frame_expected是否发过nak，防止对同一个frame_expected发出多次nak重传要求，在frame_expected改变后重置
seq_nr oldest_frame = MAX_SEQ + 1;

static boolean between(seq_nr a, seq_nr b, seq_nr c)
{ //保证a<=b<c，a为循环滑动窗口下界，c为上界(三种情况)
//1.a=0 , b=5 , c=6 (待确认：0，1，2，3，4，5)
//2.c=1 , a=5 , b=6 (待确认：5，6，7,0)
//3.b=0 , c=1 , a=2 (待确认: 2,3,4,5,6,7,0)
    if (((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a)))
        return (true);
    else
        return (false);
}

static void send_frame(frame_kind fk, seq_nr frame_nr, seq_nr frame_expected, packet buffer[])
{
    frame s;
    s.kind = fk; //帧类型，data/ack/nak三种

    if (fk == data)
        s.info = buffer[frame_nr % NR_BUFS];
    s.seq = frame_nr;
    s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1);//已确认的最后一个包是等待的包的上一个包
    if (fk == nak)
        no_nak = false; //全局变量，发完nak置false
    if (fk == nak || fk == ack)
        s.seq = 0xffffffff;
    to_physical_layer(&s);
    if (fk == data)
        start_timer(frame_nr % NR_BUFS); //
    stop_ack_timer();                    //有任何类型数据发送则停止ack计时器
}
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
static void SIGHANDLER_MYSIG_TIMEOUT(int sig, siginfo_t *info, void *data)
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
    switch (info->si_value.sival_int)
    {
        case -1:
        {
            p->event = timeout;
            break;
        }
        
        default:
        {
            p->frame_id = info->si_value.sival_int;
            p->event = timeout;
            break;
        }    
    }

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
static void SIGHANDLER_MYSIG_NETWORK_LAYER_READY(int signo)
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
    p->event = network_layer_ready;
    p->next = NULL;
}
static void wait_for_event(event_type *event) //阻塞函数，等待事件发生
{
    signal(MYSIG_FRAME_ARRIVAL, SIGHANDLER_MYSIG_FRAME_ARRIVAL);

    struct sigaction act, oact;
    act.sa_sigaction = SIGHANDLER_MYSIG_TIMEOUT;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO; //信息传递开关
    sigaction(MYSIG_TIMEOUT,&act,&act);

    signal(MYSIG_CHSUM_ERR, SIGHANDLER_MYSIG_CHSUM_ERR);
    signal(MYSIG_NETWORK_LAYER_READY, SIGHANDLER_MYSIG_NETWORK_LAYER_READY);
    if(!eventqueue)
        pause();

    event_queue p=eventqueue;    
    *event = eventqueue->event;
    if (*event == timeout)
        oldest_frame = eventqueue->frame_id;
    eventqueue=eventqueue->next;
    free(p);
    
}
/*
两个问题：oldest_frame获取和区分（ack_timeout，timeout事件）
*/
int main()
{
    const char *network_proc = "network";

    seq_nr ack_expected, next_frame_to_send;
    seq_nr frame_expected, too_far;//接收窗口的下界和上界
    int i;
    frame r;
    packet out_buf[NR_BUFS], in_buf[NR_BUFS];//发送窗口和接收窗口
    boolean arrived[NR_BUFS];//记录0~3 哪几个收到了
    seq_nr nbuffered;//已经发送了多少了包了（等待ack的包数）
    event_type event;
    //enable_network_layer();
    ack_expected = 0;
    next_frame_to_send = 0; //初始帧号为0
    frame_expected = 0;
    too_far = NR_BUFS; //too_far作为接收窗口的上界，等于frame_expect+NR_BUFS(接收缓存区的大小)
    nbuffered = 0;
    for (i = 0; i < NR_BUFS; i++)
        arrived[i] = false;

    signal(MYSIG_FRAME_ARRIVAL, SIGHANDLER_MYSIG_FRAME_ARRIVAL);
    signal(MYSIG_TIMEOUT, SIGHANDLER_MYSIG_TIMEOUT);
    signal(MYSIG_CHSUM_ERR, SIGHANDLER_MYSIG_CHSUM_ERR);
    signal(MYSIG_NETWORK_LAYER_READY, SIGHANDLER_MYSIG_NETWORK_LAYER_READY);

    while (1)
    {
        wait_for_event(&event);

        switch (event)
        {
        case network_layer_ready://同Protocol5
            nbuffered = nbuffered + 1;
            from_network_layer(&out_buf[next_frame_to_send % NR_BUFS]);
            send_frame(data, next_frame_to_send, frame_expected, out_buf);
            inc(next_frame_to_send);
            break;
        
        case frame_arrival:
            from_physical_layer(&r);
            if (r.kind == data)
            {
                if ((r.seq != frame_expected) && no_nak)//收到的帧不是想要的帧，有理由怀疑传输出错了，发送nak要求重传
                {
                    send_frame(nak, 0, frame_expected, out_buf);
                    record_err(frame_expected,rec_nak);
                }    
                else
                    start_ack_timer();//从有数据帧到来时刻开始计时，如果一段时间内都没有顺班车，就单独发一个ack
                if (between(frame_expected, r.seq, too_far) && arrived[r.seq % NR_BUFS])//重复收到错误
                    record_err(r.seq,rec_repeat);
                if (between(frame_expected, r.seq, too_far) && arrived[r.seq % NR_BUFS] == false)
                {
                    arrived[r.seq % NR_BUFS] = true;//记录该帧已经到达
                    in_buf[r.seq % NR_BUFS] = r.info; //放入接收窗中

                    while (arrived[frame_expected % NR_BUFS])//从序号低的帧开始发。直到某个没到达的帧。
                    {
                        to_network_layer(&in_buf[frame_expected % NR_BUFS]);
                        no_nak = true;                             //由于frame_expected前移了，我们又可以发nak了
                        arrived[frame_expected % NR_BUFS] = false; //清接收窗口
                        inc(frame_expected);//frame_expected前移
                        inc(too_far); //等于frame_expected+NR_BUFS，故随着frame_expected前移
                        start_ack_timer();
                    }
                }
            } // end of if(r.kind==data)
        
            /* 如果收到nak，则根据nak包中的ack，找到对方未确认的包（对方最后一个确认的包序号r.ack加1）发送 ，（这个包应该是自己待确认包之一）*/
            if ((r.kind == nak) && between(ack_expected, (r.ack + 1) % (MAX_SEQ + 1), next_frame_to_send))
            {    
                send_frame(data, (r.ack + 1) % (MAX_SEQ + 1), frame_expected, out_buf);
                record_repeat((r.ack + 1) % (MAX_SEQ + 1),1,-1,rec_nak);
            }
            /* 处理收到的ack（独立帧或数据帧捎带过来）*/
            if (!between(ack_expected, r.ack, next_frame_to_send))//收到的不在ack窗口内
                record_err(ack_expected,rec_outrange_ack);
            while(between(ack_expected, r.ack, next_frame_to_send)) //同Protocol5
            {
                nbuffered = nbuffered - 1;
                stop_timer(ack_expected % NR_BUFS); //链表中删除
                inc(ack_expected);
            }
            break;


        case cksum_err:
            record_err(frame_expected,rec_cksum_err);
            if (no_nak) //没发过nak则发nak
            {
                record_err(frame_expected,rec_cksum_nak);
                send_frame(nak, 0, frame_expected, out_buf);
            }    
            break;

        case timeout: //数据包超时则重发数据包
            record_err(oldest_frame,rec_timeout);
            record_repeat(oldest_frame,1,(frame_expected + MAX_SEQ) % (MAX_SEQ + 1),rec_timeout);
            send_frame(data, oldest_frame, frame_expected, out_buf);
            break;

        case ack_timeout: //ack超时则单独发ack包(未被捎带的情况)
            record_err((frame_expected + MAX_SEQ) % (MAX_SEQ + 1),rec_ack_timeout);
            record_repeat(-1,0,(frame_expected + MAX_SEQ) % (MAX_SEQ + 1),rec_ack_timeout);
            send_frame(ack, 0, frame_expected, out_buf);
            break;
    } //end of switch
    if (nbuffered < NR_BUFS)//最多同时发送NR_BUFS 个 包
        enable_network_layer(network_proc); //允许上层发数据
    else
        disable_network_layer(network_proc); //不允许上层发数据
    }                                // end of while
}