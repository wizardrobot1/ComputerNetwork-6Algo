#include"common.h"
#include"tools.h"
void from_network_layer(packet *p);//发送方从网络层得到纯数据包

void to_network_layer(packet *p);//接收方向网络层发送纯数据包,去掉帧的类型、发送/确认序号等控制信息

void enable_network_layer(const char* proc_name);//解除网络层阻塞,使可以产生新的network_layer_ready事件

void enable_network_layer_read(const char* proc_name);//提醒网络层读数据

void disable_network_layer(const char* proc_name);//使网络层阻塞,不再产生新的network_layer_ready事件