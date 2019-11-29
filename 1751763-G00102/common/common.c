#include"common.h"
#include"tools.h"

void from_network_layer(packet *p,int pid)//发送方从网络层得到纯数据包
{
    static int seq_PKT=0;//使用静态局部变量
    char share_file_name[MAX_FILENANE_LEN];
    int share_file=-1;
    sprintf(share_file_name, "%s%04d", NETWORK_DATALINK_SAHRE_FILE, seq_PKT);
    inc_seq_PKT(seq_PKT);
    while (share_file==-1)
    {
        share_file = open(share_file_name, O_RDONLY);
    }
    //加锁，读取文件
    //printf("set lock to %s %d\n",share_file_name,share_file);
    set_lock(share_file,F_RDLCK);
    read(share_file,p->data,MAX_PKT);
    set_lock(share_file,F_UNLCK);//读完开锁
    close(share_file);
    //向NETWORK_LAYER发送enable 信号
    sendSIG(pid,MYSIG_ENABLE_NETWORK_LAYER);
}

void to_network_layer(packet *p,int pid)//接收方向网络层发送纯数据包,去掉帧的类型、发送/确认序号等控制信息
{
    static int seq_PKT=0;//使用静态局部变量
    char share_file_name[MAX_FILENANE_LEN];
    int share_file;
    sprintf(share_file_name, "%s%04d", DATALINK_NETWORK_SAHRE_FILE, seq_PKT);
    inc_seq_PKT(seq_PKT);
    share_file = open(share_file_name, O_WRONLY | O_CREAT, 0644);

    write(share_file,p->data,MAX_PKT);
    close(share_file);
    //向NETWORK_LAYER发送enable 信号
    kill(pid,MYSIG_DATALINK_LAYER_READY);
}

void from_physical_layer(frame *f){};//接收方从物理层取得帧,帧头尾的FLAG字节、数据中的字节填充均已去掉,调用本函数前已验证过校验和，若发生错误则发送cksum_err事件，因此只有帧正确的情况下会调用本函数

void to_physical_layer(frame *f){};//发送方向物理层发送帧,帧头尾加FLAG字节、数据中进行字节填充,计算校验和放入帧尾

void start_timer(seq_nr k)
{
    
    static frame_timer *rear = mytimer;
    frame_timer *p=mytimer;
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

void set_mytimer(int sig)
{
    
}

void start_timer_signal_deal(int sig, siginfo_t *info, void *data)
{

}

void stop_timer(seq_nr k)
{

};//停止第k帧的定时器

void start_ack_timer(void){};//启动确认包定时器

void stop_ack_timer(void){};//停止确认包定时器

void enable_network_layer(void){};//解除网络层阻塞,使可以产生新的network_layer_ready事件


void disable_network_layer(void){};//使网络层阻塞,不再产生新的network_layer_ready事件