#define MAX_PKT 1024 //数据包长度，可按需改
typedef enum
{
    false,
    true
} boolen;

typedef enum
{
    false,
    true
} boolen;                    //状态枚举量(false=0/true=1)

typedef unsigned int seq_nr; //发送序号

typedef struct
{
    unsigned char data[MAX_PKT];
} packet;

typedef unsigned int seq_nr; //发送序号

typedef struct
{
    unsigned char data[MAX_PKT];
} packet; //数据包，纯数据

typedef enum
{
    data, //数据包
    ack,  //确认包
    nak   //否定确认包
} frame_kind;

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

typedef struct
{
    frame_kind kind; //帧类型
    seq_nr seq;      //发送序号
    seq_nr ack;      //接收序号
    packet info;     //数据包
} frame;