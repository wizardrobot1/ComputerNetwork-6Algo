#include"common.h"
#include"tools.h"


#define MYSIG_TIMER_START 50 //֪ͨ��ʱ���ӽ�������
#define MYSIG_TIMER_STOP 51 //֪ͨ��ʱ���ӽ���ֹͣ
#define MYTIMER_TIMEOUT_TIME 100 //��ʱ����ʱʱ�䣬��λ��ms


void set_mytimer(int sig);

void start_timer(seq_nr k);//������k֡�Ķ�ʱ??
void start_timer_signal_deal(int sig, siginfo_t *info, void *data);//������ʱ���źŴ���

void stop_timer(seq_nr k);//ֹͣ��k֡�Ķ�ʱ??
void stop_timer_signal_deal(int sig, siginfo_t *info, void *data);//ֹͣ��ʱ���źŴ���

void start_ack_timer(void);//����ȷ�ϰ���ʱ��

void stop_ack_timer(void);//ֹͣȷ�ϰ���ʱ��

