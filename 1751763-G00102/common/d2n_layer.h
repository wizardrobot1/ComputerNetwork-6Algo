#include"common.h"
#include"tools.h"
void from_network_layer(packet *p);//���ͷ��������õ������ݰ�

void to_network_layer(packet *p);//���շ�������㷢�ʹ����ݰ�,ȥ��֡�����͡�����/ȷ����ŵȿ�����Ϣ

void enable_network_layer(const char* proc_name);//������������,ʹ���Բ����µ�network_layer_ready�¼�

void enable_network_layer_read(const char* proc_name);//��������������

void disable_network_layer(const char* proc_name);//ʹ���������,���ٲ����µ�network_layer_ready�¼�