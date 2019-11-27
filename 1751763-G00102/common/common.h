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

typedef unsigned int seq_nr; //发送序号

typedef struct
{
    unsigned char data[MAX_PKT];
} packet; //数据包，纯数据

typedef enum
{
    data,     //数据包
    ack,      //确认包
    nak       //否定确认包
} frame_kind; //帧类型枚举量

typedef struct
{
    frame_kind kind; //帧类型
    seq_nr seq;      //发送序号
    seq_nr ack;      //接收序号
    packet info;     //数据包
} frame;

typedef enum
{
    frame_arrival,       //帧到达
    cksum_err,           //检验和错
    timeout,             //发送超时
    network_layer_ready, //网络层就绪
    ack_timeout          //确认包超时
} event_type;            //事件类型枚举量

#endif



void from_network_layer(packet *p,int pid);//发送方从网络层得到纯数据包

void to_network_layer(packet *p,int pid);//接收方向网络层发送纯数据包,去掉帧的类型、发送/确认序号等控制信息

void from_physical_layer(frame *f);//接收方从物理层取得帧,帧头尾的FLAG字节、数据中的字节填充均已去掉,调用本函数前已验证过校验和，若发生错误则发送cksum_err事件，因此只有帧正确的情况下会调用本函数

void to_physical_layer(frame *f);//发送方向物理层发送帧,帧头尾加FLAG字节、数据中进行字节填充,计算校验和放入帧尾


void start_timer(seq_nr k);//启动第k帧的定时器

void stop_timer(seq_nr k);//停止第k帧的定时器

void start_ack_timer(void);//启动确认包定时器

void stop_ack_timer(void);//停止确认包定时器

void enable_network_layer(void);//解除网络层阻塞,使可以产生新的network_layer_ready事件


void disable_network_layer(void);//使网络层阻塞,不再产生新的network_layer_ready事件

//------------------------------------My Signal Def-------------------------------------------
#define MYSIG_TIMEOUT SIGALRM //a) timeout 

#define MYSIG_ACKTIMEOUT SIGALRM //b) ack_timeout

#define MYSIG_CHSUM_ERR 35 //c) chsum_err 

#define MYSIG_FRAME_ARRIVAL 36 //d) frame_arrival 

#define MYSIG_NETWORK_LAYER_READY 37 //e) network_layer_ready

#define MYSIG_ENABLE_NETWORK_LAYER 38 //f) enable_network_layer

#define MYSIG_DISABLE_NETWORK_LAYER 39 //g) disable_network_layer

#define MYSIG_DATALINK_LAYER_READY 40 // datalink_layer_ready //网络层可以收数据了

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


