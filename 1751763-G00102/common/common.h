#ifndef __COMMON__H
#define __COMMON__H
#include "time.h"
#include "stdio.h"
#include <stdlib.h>  
#include <string.h>  
#include <sys/types.h>  
#include <sys/wait.h>
#include <sys/socket.h>  
#include <netinet/in.h> 
#include <sys/errno.h> 
#endif

#define MAX_PKT 1024 //数据包长度

#ifndef __COMMON__T
#define __COMMON__T
typedef enum
{
    false,
    true
} boolen;


typedef unsigned int seq_nr; //发送序号

typedef struct
{
    unsigned char data[MAX_PKT];
} packet;


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
} event_type;

#endif

void wait_for_event( event_type *event);
//阻塞函数，等待事件发生
void from_network_layer(packet *p);
//发送方从网络层得到纯数据包
void to_network_layer(packet *p);
//接收方向网络层发送纯数据包
//去掉帧的类型、发送/确认序号等控制信息


void from_physical_layer(packet *p);
//接收方从物理层取得帧
//帧头尾的FLAG字节、数据中的字节填充均已去掉
//调用本函数前已验证过校验和，若发生错误则发送cksum_err事件，因此只有帧正确的情况下会调用本函数

void to_physical_layer(packet *p);
//发送方向物理层发送帧
//帧头尾加FLAG字节、数据中进行字节填充
//计算校验和放入帧尾


void start_timer(seq_nr k);
//启动第k帧的定时器
void stop_timer(seq_nr k);
//停止第k帧的定时器
void start_ack_timer(void);
//启动确认包定时器
void stop_ack_timer(void);
//停止确认包定时器
void enable_network_layer(void);
//解除网络层阻塞
//使可以产生新的network_layer_ready事件
//解除网络层阻塞
//使可以产生新的network_layer_ready事件
void disable_network_layer(void);
//使网络层阻塞
//不再产生新的network_layer_ready事件
//使网络层阻塞
//不再产生新的network_layer_ready事件

/*不同算法的MAX_SEQ不同，故不能宏定义
#define inc(k) if(k<MAX_SEQ) k=k+1; else k=0;
//使k在[0 ~ MAX_SEQ-1]间循环增长
//如果MAX_SEQ=1，则0/1互换
*/

#define MYSIG_TIMEOUT SIGALARM//a) timeout 用 SIGALRM 信号，精度在 ms 级（不要用 alarm 函数）

#define MYSIG_ACKTIMEOUT SIGALRM//b) ack_timeout 也用 SIGALRM 信号

#define MYSIG_CHSUM_ERR 35//c) chsum_err 用自定义的 35 号信号

#define MYSIG_FRAME_ARRIVAL 36//d) frame_arrival 用自定义的 36 号信号

#define MYSIG_NETWORK_LAYER_READY 37//e) network_layer_ready 用自定义的 37 号信号

#define MYSIG_ENABLE_NETWORK_LAYER 38//f) enable_network_layer 由 SDL 向 SNL 发送 38 号信号

#define MYSIG_DISABLE_NETWORK_LAYER 39//g) disable_network_layer 由 SDL 向 SNL 发送 39 号信号
