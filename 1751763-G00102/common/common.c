#include"common.h"
#include"tools.h"
void from_network_layer(packet *p,int pid)//发送方从网络层得到纯数据包
{
    static int seq_PKT=0;//使用静态局部变量
    char share_file_name[MAX_FILENANE_LEN];
    int share_file;
    sprintf(share_file_name, "%s%04d", NETWORK_DATALINK_SAHRE_FILE, seq_PKT);
    inc_seq_PKT(seq_PKT);
    share_file = open(share_file_name, O_RDONLY);

    //加锁，读取文件
    set_lock(share_file,F_WRLCK);
    read(share_file,p->data,MAX_PKT);
    set_lock(share_file,F_UNLCK);//读完开锁
    //向NETWORK_LAYER发送enable 信号
    kill(pid,MYSIG_ENABLE_NETWORK_LAYER);
}

void to_network_layer(packet *p,int pid)//接收方向网络层发送纯数据包,去掉帧的类型、发送/确认序号等控制信息
{
    static int seq_PKT=0;//使用静态局部变量
    char share_file_name[MAX_FILENANE_LEN];
    int share_file;
    sprintf(share_file_name, "%s%04d", DATALINK_NETWORK_SAHRE_FILE, seq_PKT);
    inc_seq_PKT(seq_PKT);
    share_file = open(share_file_name, O_WRONLY | O_CREAT, 0644);

    write(share_file,p->data,MAX_PKT);

    //向NETWORK_LAYER发送enable 信号
    kill(pid,MYSIG_DATALINK_LAYER_READY);
}
