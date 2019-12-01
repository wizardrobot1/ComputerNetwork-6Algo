#include "timer.h"

static int ppid=0;
static int timer_cpid=0;//��¼�ӽ���pid
static int ack_timer_cpid=0;
static frame_timer *mytimer = NULL; //��ʱ��������λms
static frame_timer *ack_timer = NULL;


void start_timer(seq_nr k)
{
    
    static frame_timer *rear = NULL;
    int frame_id;

    if (!timer_cpid)
    {
        ppid = getpid();
        timer_cpid = fork();
    }

    if(!timer_cpid)
    {
        mytimer=(frame_timer*)malloc(sizeof(frame_timer));
        mytimer->next = NULL;
        rear=mytimer;

        struct sigaction act_start, act_stop, oact_start, oact_stop;

        act_start.sa_sigaction=start_timer_signal_deal;
        sigemptyset(&act_start.sa_mask);
        act_start.sa_flags = SA_SIGINFO;//��Ϣ���ݿ���

        act_stop.sa_sigaction=stop_timer_signal_deal;
        sigemptyset(&act_stop.sa_mask);
        act_stop.sa_flags = SA_SIGINFO;//��Ϣ���ݿ���

        struct itimerval new_value, old_value;
        new_value.it_value.tv_sec = 0;
        new_value.it_value.tv_usec = 1;
        new_value.it_interval.tv_sec = 0;
        new_value.it_interval.tv_usec = 1;
        setitimer(ITIMER_REAL, &new_value, &old_value);//�ӽ���ÿ�������Լ�����һ��sigalarm

        signal(SIGALRM,set_mytimer);

        sigaction(MYSIG_TIMER_START,&act_start,&oact_start);
        sigaction(MYSIG_TIMER_STOP,&act_stop,&oact_stop);
        while(1)
        {
            ;
        }
    }
    else//�����̷��ͼ�ʱ�ź�
    {
        union sigval mysigval;
        mysigval.sival_int = k;
        sigqueue(timer_cpid,MYSIG_TIMER_START,mysigval);
    }
    return;

};//������k֡�Ķ�ʱ��

void set_mytimer(int sig)//ÿ���봦���ʱ���������
{
    int i = 0;
    frame_timer *p ;
    while (mytimer && mytimer->msec == 1)
    {
        p = mytimer;
        mytimer = mytimer->next;
        free(p);
        union sigval mysigval;
        mysigval.sival_int = i;
        sigqueue(ppid, MYSIG_TIMEOUT, mysigval);
        ++i;
    }
    if(mytimer)
        mytimer->msec--;
}

void start_timer_signal_deal(int sig, siginfo_t *info, void *data)//������ʱ���źŴ���
{
    int frame_id = info->si_value.sival_int; //֡��
    frame_timer *p = mytimer, *q = p;
    int msec = MYTIMER_TIMEOUT_TIME;
    while(p)
    {
        q = p;
        msec = msec - p->msec;
        p = p->next;
    }
    if(q)
    {
        q->next=(frame_timer *)malloc(sizeof(frame_timer));
        q->next->frame_id = frame_id;
        q->next->msec = msec;
        q->next->next=NULL;
    }
    else
    {
        q=(frame_timer *)malloc(sizeof(frame_timer));
        q->frame_id = frame_id;
        q->msec = msec;
        q->next=NULL;
        mytimer = q;
    }
    
}

void stop_timer(seq_nr k)
{
    union sigval mysigval;
    mysigval.sival_int = k;
    sigqueue(timer_cpid, MYSIG_TIMER_STOP, mysigval);
}

void stop_timer_signal_deal(int sig, siginfo_t *info, void *data)//ֹͣ��ʱ���źŴ���
{
    int frame_id = info->si_value.sival_int; //֡��
    frame_timer *p = mytimer, *q = p;
    int msec;
    while(p)
    {
        if (p->frame_id == frame_id)
        {
            msec = p->msec;
            p = p->next;
             p->msec += msec;
            break;
        }    
        else
            p = p->next;   
    }

}

void start_ack_timer(void)//����ȷ�ϰ���ʱ��
{
    if(!ack_timer_cpid)
    {
        if(!ppid)
            ppid = getpid();
        ack_timer_cpid = fork();
    }    

    if(ack_timer_cpid)
    {
        union sigval mysigval;
        mysigval.sival_int = -1;
        sigqueue(ack_timer_cpid, MYSIG_ACK_TIMER_START, mysigval);
        return;
    }
    else
    {
        struct itimerval new_value, old_value;
        new_value.it_value.tv_sec = 0;
        new_value.it_value.tv_usec = 1;
        new_value.it_interval.tv_sec = 0;
        new_value.it_interval.tv_usec = 1;
        setitimer(ITIMER_REAL, &new_value, &old_value);//�ӽ���ÿ1�������Լ�����һ��sigalarm

        signal(SIGALRM,set_acktimer);


        struct sigaction act_start, act_stop, oact_start, oact_stop;

        act_start.sa_sigaction=start_ack_timer_signal_deal;
        sigemptyset(&act_start.sa_mask);
        act_start.sa_flags = SA_SIGINFO;//��Ϣ���ݿ���

        act_stop.sa_sigaction=stop_ack_timer_signal_deal;
        sigemptyset(&act_stop.sa_mask);
        act_stop.sa_flags = SA_SIGINFO;//��Ϣ���ݿ���

        sigaction(MYSIG_ACK_TIMER_START,&act_start,&oact_start);
        sigaction(MYSIG_ACK_TIMER_STOP,&act_stop,&oact_stop);
    }
    


    while(1)
        ;
}

void set_acktimer(int sig)
{
    if(ack_timer&&ack_timer->msec == 1)
    {
        union sigval mysigval;
        mysigval.sival_int = -1;
        sigqueue(ppid, MYSIG_TIMEOUT, mysigval);
    }
    if(ack_timer)
        ack_timer->msec--;
}

void start_ack_timer_signal_deal(int sig, siginfo_t *info, void *data)//������ʱ���źŴ���
{
    if(!ack_timer)
        ack_timer = (frame_timer *)malloc(sizeof(frame_timer));
    ack_timer->msec = MYTIMER_TIMEOUT_TIME;
}



void stop_ack_timer(void)//ֹͣȷ�ϰ���ʱ��
{
    union sigval mysigval;
    mysigval.sival_int = -1;
    sigqueue(ack_timer_cpid, MYSIG_ACK_TIMER_STOP, mysigval);
}

void stop_ack_timer_signal_deal(int sig, siginfo_t *info, void *data)//������ʱ���źŴ���
{
    ack_timer->msec = -1;
}