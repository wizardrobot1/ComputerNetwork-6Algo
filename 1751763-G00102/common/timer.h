#include"common.h"
#include"tools.h"


#define MYSIG_TIMER_START 50 //通知定时器子进程启动
#define MYSIG_TIMER_STOP 51 //通知定时器子进程停止
#define MYTIMER_TIMEOUT_TIME 100 //定时器超时时间，单位：ms


void set_mytimer(int sig);

void start_timer(seq_nr k);//启动第k帧的定时??
void start_timer_signal_deal(int sig, siginfo_t *info, void *data);//启动定时器信号处理

void stop_timer(seq_nr k);//停止第k帧的定时??
void stop_timer_signal_deal(int sig, siginfo_t *info, void *data);//停止定时器信号处理

void start_ack_timer(void);//启动确认包定时器

void stop_ack_timer(void);//停止确认包定时器

