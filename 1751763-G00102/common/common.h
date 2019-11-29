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
} boolen; //状态枚举量(false=0/true=1)

typedef unsigned int seq_nr; //发送序??

typedef struct
{
    unsigned char data[MAX_PKT];
} packet; //数据包，纯数??

typedef enum
{
    data,     //数据??
    ack,      //确认??
    nak       //否定确认??
} frame_kind; //帧类型枚举量

typedef struct
{
    frame_kind kind; //帧类??
    seq_nr seq;      //发送序??
    seq_nr ack;      //接收序号
    packet info;     //数据??
} frame;

typedef enum
{
    frame_arrival,       //帧到??
    cksum_err,           //检验和??
    timeout,             //发送超??
    network_layer_ready, //网络层就??
    ack_timeout          //确认包超??
} event_type;            //事件类型枚举??

typedef struct
{
    int frame_id;
    int msec;
    frame_second *next;
} frame_timer;


#endif

//---------------------------------Func Def

void from_network_layer(packet *p);//发送方从网络层得到纯数据包

void to_network_layer(packet *p);//接收方向网络层发送纯数据??,去掉帧的类型、发??/确认序号等控制信??

void from_physical_layer(frame *f);//接收方从物理层取得帧,帧头尾的FLAG字节、数据中的字节填充均已去??,调用本函数前已验证过校验和，若发生错误则发送cksum_err事件，因此只有帧正确的情况下会调用本函数

void to_physical_layer(frame *f);//发送方向物理层发送帧,帧头尾加FLAG字节、数据中进行字节填充,计算校验和放入帧??

void set_mytimer(int sig);

void start_timer(seq_nr k);//启动第k帧的定时??
void start_timer_signal_deal(int sig, siginfo_t *info, void *data);//启动定时器信号处理

void stop_timer(seq_nr k);//停止第k帧的定时??
void stop_timer_signal_deal(int sig, siginfo_t *info, void *data);//停止定时器信号处理

void start_ack_timer(void);//启动确认包定时器

void stop_ack_timer(void);//停止确认包定时器

void enable_network_layer(const char* proc_name);//解除网络层阻??,使可以产生新的network_layer_ready事件

void enable_network_layer_read(const char* proc_name);//提醒网络层读数据
void disable_network_layer(const char* proc_name);//使网络层阻塞,不再产生新的network_layer_ready事件

//------------------------------------My Signal Def-------------------------------------------
#define MYSIG_TIMEOUT SIGALRM //a) timeout 

#define MYSIG_ACKTIMEOUT SIGALRM //b) ack_timeout

#define MYSIG_CHSUM_ERR 35 //c) chsum_err 

#define MYSIG_FRAME_ARRIVAL 36 //d) frame_arrival 

#define MYSIG_NETWORK_LAYER_READY 37 //e) network_layer_ready

#define MYSIG_ENABLE_NETWORK_LAYER 38 //f) enable_network_layer

#define MYSIG_DISABLE_NETWORK_LAYER 39 //g) disable_network_layer

#define MYSIG_DATALINK_LAYER_READY 40 // datalink_layer_ready //网络层可以收数据??

#define MYSIG_TIMER_START 50 //通知定时器子进程启动

#define MYSIG_TIMER_STOP 51 //通知定时器子进程启动


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

#define MYTIMER_TIMEOUT_TIME 100 //定时器超时时间，单位：ms

