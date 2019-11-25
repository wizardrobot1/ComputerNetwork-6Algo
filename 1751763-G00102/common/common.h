#define MAX_PKT 1024 //数据包长度，可按需改
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


void wait_for_event( event_type *event);
//阻塞函数，等待事件发生
void from_network_layer(packet *p);
//发送方从网络层得到纯数据包
void to_network_layer(packet *p);
//接收方向网络层发送纯数据包
//去掉帧的类型、发送/确认序号等控制信息
//接收方向网络层发送纯数据包
//去掉帧的类型、发送/确认序号等控制信息
void from_physical_layer(packet *p);
//接收方从物理层取得帧
//帧头尾的FLAG字节、数据中的字节填充均已去掉
//调用本函数前已验证过校验和，若发生错误
//则发送cksum_err事件，因此只有帧正确的
//情况下会调用本函数

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
#define inc(k) if(k<MAX_SEQ) k=k+1; else k=0;
//使k在[0 ~ MAX_SEQ-1]间循环增长
//如果MAX_SEQ=1，则0/1互换