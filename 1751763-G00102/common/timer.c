#include "timer.h"

static int ppid=0;
static int timer_cpid=0;//记录子进程pid
static int ack_timer_cpid=0;
static frame_timer *mytimer = NULL; //定时器链表，单位ms
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
        act_start.sa_flags = SA_SIGINFO;//信息传递开关

        act_stop.sa_sigaction=stop_timer_signal_deal;
        sigemptyset(&act_stop.sa_mask);
        act_stop.sa_flags = SA_SIGINFO;//信息传递开关

        struct itimerval new_value, old_value;
        new_value.it_value.tv_sec = 0;
        new_value.it_value.tv_usec = 1;
        new_value.it_interval.tv_sec = 0;
        new_value.it_interval.tv_usec = 1;
        setitimer(ITIMER_REAL, &new_value, &old_value);//子进程每毫秒向自己发送一个sigalarm

        signal(SIGALRM,set_mytimer);

        sigaction(MYSIG_TIMER_START,&act_start,&oact_start);
        sigaction(MYSIG_TIMER_STOP,&act_stop,&oact_stop);
        while(1)
        {
            ;
        }
    }
    else//父进程发送计时信号
    {
        union sigval mysigval;
        mysigval.sival_int = k;
        sigqueue(timer_cpid,MYSIG_TIMER_START,mysigval);
    }
    return;

};//启动第k帧的定时器

void set_mytimer(int sig)//每毫秒处理计时器链表操作
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

void start_timer_signal_deal(int sig, siginfo_t *info, void *data)//启动定时器信号处理
{
    int frame_id = info->si_value.sival_int; //帧号
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

void stop_timer_signal_deal(int sig, siginfo_t *info, void *data)//停止定时器信号处理
{
    int frame_id = info->si_value.sival_int; //帧号
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

void start_ack_timer(void)//启动确认包定时器
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
        setitimer(ITIMER_REAL, &new_value, &old_value);//子进程每1毫秒向自己发送一个sigalarm

        signal(SIGALRM,set_acktimer);


        struct sigaction act_start, act_stop, oact_start, oact_stop;

        act_start.sa_sigaction=start_ack_timer_signal_deal;
        sigemptyset(&act_start.sa_mask);
        act_start.sa_flags = SA_SIGINFO;//信息传递开关

        act_stop.sa_sigaction=stop_ack_timer_signal_deal;
        sigemptyset(&act_stop.sa_mask);
        act_stop.sa_flags = SA_SIGINFO;//信息传递开关

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

void start_ack_timer_signal_deal(int sig, siginfo_t *info, void *data)//启动定时器信号处理
{
    if(!ack_timer)
        ack_timer = (frame_timer *)malloc(sizeof(frame_timer));
    ack_timer->msec = MYTIMER_TIMEOUT_TIME;
}



void stop_ack_timer(void)//停止确认包定时器
{
    union sigval mysigval;
    mysigval.sival_int = -1;
    sigqueue(ack_timer_cpid, MYSIG_ACK_TIMER_STOP, mysigval);
}

void stop_ack_timer_signal_deal(int sig, siginfo_t *info, void *data)//启动定时器信号处理
{
    ack_timer->msec = -1;
}