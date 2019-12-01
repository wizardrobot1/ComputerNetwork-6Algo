#include "../common/common.h"
#include "../common/tools.h"
#include "../common/d2n_layer.h"
#include "../common/savelog.h"
#define MAX_SEQ 7 //����2^n -1
#define inc(k)           \
    if (k < MAX_SEQ + 1) \
        k = k + 1;       \
    else                 \
        k = 0; //k=0~7 ���ڵĴ�С��MAX_SEQ+1 , ֡�����0~MAX_SEQ
#define NR_BUFS ((MAX_SEQ + 1) / 2)//���ǿ�ͬʱ����֡��������Ҳ�ǽ��շ��Ļ�������С
boolean no_nak = true;//��¼���ڵ�ǰframe_expected�Ƿ񷢹�nak����ֹ��ͬһ��frame_expected�������nak�ش�Ҫ����frame_expected�ı������
seq_nr oldest_frame = MAX_SEQ + 1;

static boolean between(seq_nr a, seq_nr b, seq_nr c)
{ //��֤a<=b<c��aΪѭ�����������½磬cΪ�Ͻ�(�������)
//1.a=0 , b=5 , c=6 (��ȷ�ϣ�0��1��2��3��4��5)
//2.c=1 , a=5 , b=6 (��ȷ�ϣ�5��6��7,0)
//3.b=0 , c=1 , a=2 (��ȷ��: 2,3,4,5,6,7,0)
    if (((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a)))
        return (true);
    else
        return (false);
}

static void send_frame(frame_kind fk, seq_nr frame_nr, seq_nr frame_expected, packet buffer[])
{
    frame s;
    s.kind = fk; //֡���ͣ�data/ack/nak����

    if (fk == data)
        s.info = buffer[frame_nr % NR_BUFS];
    s.seq = frame_nr;
    s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1);//��ȷ�ϵ����һ�����ǵȴ��İ�����һ����
    if (fk == nak)
        no_nak = false; //ȫ�ֱ���������nak��false
    if (fk == nak || fk == ack)
        s.seq = 0xffffffff;
    to_physical_layer(&s);
    if (fk == data)
        start_timer(frame_nr % NR_BUFS); //
    stop_ack_timer();                    //���κ��������ݷ�����ֹͣack��ʱ��
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
static void wait_for_event(event_type *event) //�����������ȴ��¼�����
{
    signal(MYSIG_FRAME_ARRIVAL, SIGHANDLER_MYSIG_FRAME_ARRIVAL);

    struct sigaction act, oact;
    act.sa_sigaction = SIGHANDLER_MYSIG_TIMEOUT;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO; //��Ϣ���ݿ���
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
�������⣺oldest_frame��ȡ�����֣�ack_timeout��timeout�¼���
*/
int main()
{
    const char *network_proc = "network";

    seq_nr ack_expected, next_frame_to_send;
    seq_nr frame_expected, too_far;//���մ��ڵ��½���Ͻ�
    int i;
    frame r;
    packet out_buf[NR_BUFS], in_buf[NR_BUFS];//���ʹ��ںͽ��մ���
    boolean arrived[NR_BUFS];//��¼0~3 �ļ����յ���
    seq_nr nbuffered;//�Ѿ������˶����˰��ˣ��ȴ�ack�İ�����
    event_type event;
    //enable_network_layer();
    ack_expected = 0;
    next_frame_to_send = 0; //��ʼ֡��Ϊ0
    frame_expected = 0;
    too_far = NR_BUFS; //too_far��Ϊ���մ��ڵ��Ͻ磬����frame_expect+NR_BUFS(���ջ������Ĵ�С)
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
        case network_layer_ready://ͬProtocol5
            nbuffered = nbuffered + 1;
            from_network_layer(&out_buf[next_frame_to_send % NR_BUFS]);
            send_frame(data, next_frame_to_send, frame_expected, out_buf);
            inc(next_frame_to_send);
            break;
        
        case frame_arrival:
            from_physical_layer(&r);
            if (r.kind == data)
            {
                if ((r.seq != frame_expected) && no_nak)//�յ���֡������Ҫ��֡�������ɻ��ɴ�������ˣ�����nakҪ���ش�
                {
                    send_frame(nak, 0, frame_expected, out_buf);
                    record_err(frame_expected,rec_nak);
                }    
                else
                    start_ack_timer();//��������֡����ʱ�̿�ʼ��ʱ�����һ��ʱ���ڶ�û��˳�೵���͵�����һ��ack
                if (between(frame_expected, r.seq, too_far) && arrived[r.seq % NR_BUFS])//�ظ��յ�����
                    record_err(r.seq,rec_repeat);
                if (between(frame_expected, r.seq, too_far) && arrived[r.seq % NR_BUFS] == false)
                {
                    arrived[r.seq % NR_BUFS] = true;//��¼��֡�Ѿ�����
                    in_buf[r.seq % NR_BUFS] = r.info; //������մ���

                    while (arrived[frame_expected % NR_BUFS])//����ŵ͵�֡��ʼ����ֱ��ĳ��û�����֡��
                    {
                        to_network_layer(&in_buf[frame_expected % NR_BUFS]);
                        no_nak = true;                             //����frame_expectedǰ���ˣ������ֿ��Է�nak��
                        arrived[frame_expected % NR_BUFS] = false; //����մ���
                        inc(frame_expected);//frame_expectedǰ��
                        inc(too_far); //����frame_expected+NR_BUFS��������frame_expectedǰ��
                        start_ack_timer();
                    }
                }
            } // end of if(r.kind==data)
        
            /* ����յ�nak�������nak���е�ack���ҵ��Է�δȷ�ϵİ����Է����һ��ȷ�ϵİ����r.ack��1������ ���������Ӧ�����Լ���ȷ�ϰ�֮һ��*/
            if ((r.kind == nak) && between(ack_expected, (r.ack + 1) % (MAX_SEQ + 1), next_frame_to_send))
            {    
                send_frame(data, (r.ack + 1) % (MAX_SEQ + 1), frame_expected, out_buf);
                record_repeat((r.ack + 1) % (MAX_SEQ + 1),1,-1,rec_nak);
            }
            /* �����յ���ack������֡������֡�Ӵ�������*/
            if (!between(ack_expected, r.ack, next_frame_to_send))//�յ��Ĳ���ack������
                record_err(ack_expected,rec_outrange_ack);
            while(between(ack_expected, r.ack, next_frame_to_send)) //ͬProtocol5
            {
                nbuffered = nbuffered - 1;
                stop_timer(ack_expected % NR_BUFS); //������ɾ��
                inc(ack_expected);
            }
            break;


        case cksum_err:
            record_err(frame_expected,rec_cksum_err);
            if (no_nak) //û����nak��nak
            {
                record_err(frame_expected,rec_cksum_nak);
                send_frame(nak, 0, frame_expected, out_buf);
            }    
            break;

        case timeout: //���ݰ���ʱ���ط����ݰ�
            record_err(oldest_frame,rec_timeout);
            record_repeat(oldest_frame,1,(frame_expected + MAX_SEQ) % (MAX_SEQ + 1),rec_timeout);
            send_frame(data, oldest_frame, frame_expected, out_buf);
            break;

        case ack_timeout: //ack��ʱ�򵥶���ack��(δ���Ӵ������)
            record_err((frame_expected + MAX_SEQ) % (MAX_SEQ + 1),rec_ack_timeout);
            record_repeat(-1,0,(frame_expected + MAX_SEQ) % (MAX_SEQ + 1),rec_ack_timeout);
            send_frame(ack, 0, frame_expected, out_buf);
            break;
    } //end of switch
    if (nbuffered < NR_BUFS)//���ͬʱ����NR_BUFS �� ��
        enable_network_layer(network_proc); //�����ϲ㷢����
    else
        disable_network_layer(network_proc); //�������ϲ㷢����
    }                                // end of while
}