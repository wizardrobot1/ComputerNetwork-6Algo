#include"common.h"
#include"tools.h"

//static frame_timer *mytimer = NULL;





void from_physical_layer(frame *f){};//接收方从物理层取得帧,帧头尾的FLAG字节、数据中的字节填充均已去掉,调用本函数前已验证过校验和，若发生错误则发送cksum_err事件，因此只有帧正确的情况下会调用本函数

void to_physical_layer(frame *f){};//发送方向物理层发送帧,帧头尾加FLAG字节、数据中进行字节填充,计算校验和放入帧尾

void start_timer(seq_nr k)
{


};//启动第k帧的定时器

void stop_timer(seq_nr k)
{

};//停止第k帧的定时器

void start_ack_timer(void){};//启动确认包定时器

void stop_ack_timer(void){};//停止确认包定时器

