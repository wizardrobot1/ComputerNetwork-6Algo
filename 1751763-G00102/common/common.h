#ifndef __COMMON__H
#define __COMMON__H
#include <sys/time.h>
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
    ack_timeout,          //ȷ�ϰ���ʱ
    datalink_layer_ready
} event_type;            //�¼�����ö����

typedef struct frame_timer
{
    int frame_id;
    int msec;
    struct frame_timer *next;
} frame_timer;

typedef struct event_queue_node
{
    event_type event;
    seq_nr frame_id;
    struct  event_queue_node *next;
}*event_queue;

#endif

//------------------------------------My Signal Def-------------------------------------------
#define MYSIG_TIMEOUT SIGALRM //a) timeout 

#define MYSIG_ACKTIMEOUT SIGALRM //b) ack_timeout

#define MYSIG_CHSUM_ERR 35 //c) chsum_err 

#define MYSIG_FRAME_ARRIVAL 36 //d) frame_arrival 

#define MYSIG_NETWORK_LAYER_READY 37 //e) network_layer_ready

#define MYSIG_ENABLE_NETWORK_LAYER 38 //f) enable_network_layer

#define MYSIG_DISABLE_NETWORK_LAYER 39 //g) disable_network_layer

#define MYSIG_DATALINK_LAYER_READY 40 // datalink_layer_ready //����������������

#define MYSIG_ENABLE_DATALINK_LAYER 41 // enable_datalink_layer

#define MYSIG_DISABLE_DATALINK_LAYER 42 // disable_datalink_layer

#define MYSIG_PHYSICAL_LAYER_READY 43 // physical_layer_ready //��·����Է�������

#define MYSIG_ENABLE_PHYSICAL_LAYER 44 // enable_physical_layer

#define MYSIG_DISABLE_PHYSICAL_LAYER 45 // disable_physical_layer
//------------------------------------Share file name Def-------------------------------------------

#define NETWORK_DATALINK_SHARE_FILE "network_datalink.share."
#define DATALINK_NETWORK_SHARE_FILE "datalink_network.share."
#define PHYSICAL_DATALINK_SHARE_FILE "physical_datalink.share."
#define DATALINK_PHYSICAL_SHARE_FILE "datalink_physical.share."
//------------------------------------Share file seq_inc_fun Def
#define inc_seq_PKT(k) \
    if (k < 999)     \
        k = k + 1;     \
    else               \
        k = 0;

#define MAX_FILENANE_LEN 256

#define MYTIMER_TIMEOUT_TIME 100 //��ʱ����ʱʱ�䣬��λ��ms

//-----------------------------------log path-----------------------------------------------------
#define LOG_PATH "./log"
