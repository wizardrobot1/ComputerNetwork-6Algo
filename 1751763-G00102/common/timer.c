#include "timmer.h"

static int cpid=0;//记录子进程pid
static frame_timer *mytimer = NULL; //定时器链表，单位ms


void start_timer(seq_nr k)
{
    
    static frame_timer *rear = mytimer;
    int frame_id;

    if (cpid == 0)
        cpid = fork();
    if(!cpid)
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

        sigaction(MYSIG_TIMER_START,&act_start,&oact_start));
        sigaction(MYSIG_TIMER_STOP,&act_stop,&oact_stop));
        while(1)
        {
            ;
        }
    }
    else//父进程发送计时信号
    {
        union sigval mysigval;
        mysigval.sival_int = k;
        sigqueue(cpid,MYSIG_TIMER_START,mysigval);
    }
    return;

};//启动第k帧的定时器

void set_mytimer(int sig)//每毫秒处理计时器链表操作
{
    frame_timer *p;
    while (mytimer && !mytimer->msec)
    {
        p=mytimer;
        mytimer=mytimer->next;
        free(p);
    }
    if(mytimer)
        --mytimer->msec;
}