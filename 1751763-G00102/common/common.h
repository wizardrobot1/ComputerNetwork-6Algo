#ifndef __COMMON__H
#define __COMMON__H
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/file.h>  
#include <sys/stat.h> 
#include <netinet/in.h>
#include <sys/errno.h>
#endif

#define MAX_PKT 1024

#ifndef __COMMON__T
#define __COMMON__T

typedef enum
{
    false,
    true
} boolean; //״̬ö����(false=0/true=1)

typedef unsigned int seq_nr; //�������

typedef struct
{
    unsigned char data[MAX_PKT];
} packet; //���ݰ���������

typedef enum
{
    data,     //���ݰ�
    ack,      //ȷ�ϰ�
    nak       //��ȷ�ϰ�
} frame_kind; //֡����ö����

typedef struct
{
    frame_kind kind; //֡����
    seq_nr seq;      //�������
    seq_nr ack;      //�������
    packet info;     //���ݰ�
} frame;

typedef enum
{
    frame_arrival,       //֡����
    cksum_err,           //����ʹ�
    timeout,             //���ͳ�ʱ
    network_layer_ready, //��������
    ack_timeout          //ȷ�ϰ���ʱ
} event_type;            //�¼�����ö����

typedef struct
{
    int frame_id;
    int sec;
    frame_second *next;
} frame_timer;


#endif

//---------------------------------Func Def


void from_physical_layer(frame *f);//���շ��������ȡ��֡,֡ͷβ��FLAG�ֽڡ������е��ֽ�������ȥ��,���ñ�����ǰ����֤��У��ͣ���������������cksum_err�¼������ֻ��֡��ȷ������»���ñ�����

void to_physical_layer(frame *f);//���ͷ�������㷢��֡,֡ͷβ��FLAG�ֽڡ������н����ֽ����,����У��ͷ���֡β


void start_timer(seq_nr k);//������k֡�Ķ�ʱ��

void stop_timer(seq_nr k);//ֹͣ��k֡�Ķ�ʱ��

void start_ack_timer(void);//����ȷ�ϰ���ʱ��

void stop_ack_timer(void);//ֹͣȷ�ϰ���ʱ��


//------------------------------------My Signal Def-------------------------------------------
#define MYSIG_TIMEOUT SIGALRM //a) timeout 

#define MYSIG_ACKTIMEOUT SIGALRM //b) ack_timeout

#define MYSIG_CHSUM_ERR 35 //c) chsum_err 

#define MYSIG_FRAME_ARRIVAL 36 //d) frame_arrival 

#define MYSIG_NETWORK_LAYER_READY 37 //e) network_layer_ready

#define MYSIG_ENABLE_NETWORK_LAYER 38 //f) enable_network_layer

#define MYSIG_DISABLE_NETWORK_LAYER 39 //g) disable_network_layer

#define MYSIG_DATALINK_LAYER_READY 40 // datalink_layer_ready //����������������

#define MYSIG_TIMER_START 50 //֪ͨ��ʱ���ӽ�������

#define MYSIG_TIMER_STOP 51 //֪ͨ��ʱ���ӽ�������


//------------------------------------Share file name Def-------------------------------------------

#define NETWORK_DATALINK_SAHRE_FILE "network_datalink.share."
#define DATALINK_NETWORK_SAHRE_FILE "datalink_network.share."
//------------------------------------Share file seq_inc_fun Def
#define inc_seq_PKT(k) \
    if (k < 10000)     \
        k = k + 1;     \
    else               \
        k = 0;

#define MAX_FILENANE_LEN 256

#define MYTIMER_TIMEOUT_TIME 1 //��ʱ����ʱʱ�䣬��λ��s
