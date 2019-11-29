#include "timmer.h"

static int cpid=0;//��¼�ӽ���pid
static frame_timer *mytimer = NULL; //��ʱ��������λms


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

        sigaction(MYSIG_TIMER_START,&act_start,&oact_start));
        sigaction(MYSIG_TIMER_STOP,&act_stop,&oact_stop));
        while(1)
        {
            ;
        }
    }
    else//�����̷��ͼ�ʱ�ź�
    {
        union sigval mysigval;
        mysigval.sival_int = k;
        sigqueue(cpid,MYSIG_TIMER_START,mysigval);
    }
    return;

};//������k֡�Ķ�ʱ��

void set_mytimer(int sig)//ÿ���봦���ʱ���������
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