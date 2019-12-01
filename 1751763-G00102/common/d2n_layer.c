#include"d2n_layer.h"

void enable_network_layer(const char* proc_name)
{
    static pid=-1;
    while (pid==-1)//如果求过就不用再求了。
        pid=get_first_pid(proc_name);
    sendSIG(pid,MYSIG_ENABLE_NETWORK_LAYER);

};//解除网络层阻塞,使可以产生新的network_layer_ready事件

void disable_network_layer(const char* proc_name)
{
    static pid=-1;
    while (pid==-1)//如果求过就不用再求了。
        pid=get_first_pid(proc_name);
    sendSIG(pid,MYSIG_DISABLE_NETWORK_LAYER);
};//使网络层阻塞,不再产生新的network_layer_ready事件

int from_network_layer(packet *p)//发送方从网络层得到纯数据包
{
    static int seq_PKT=0;//使用静态局部变量
    char share_file_name[MAX_FILENANE_LEN];
    int share_file=-1;
    sprintf(share_file_name, "%s%04d", NETWORK_DATALINK_SHARE_FILE, seq_PKT);
    inc_seq_PKT(seq_PKT);
    printf("open share...\n");
    while (share_file==-1)
        share_file = open(share_file_name, O_RDONLY);
    printf("open share ok %d\n",seq_PKT);
    //加锁，读取文件
    //printf("set lock to %s %d\n",share_file_name,share_file);
    set_lock(share_file,F_RDLCK);
    read(share_file,p->data,MAX_PKT);
    set_lock(share_file,F_UNLCK);//读完开锁
    close(share_file);
    printf("close share ok\n");
    //向NETWORK_LAYER发送enable 信号分离出去 由enable_network_layer完成

    return 0;
}
void to_network_layer(packet *p)//接收方向网络层发送纯数据包,去掉帧的类型、发送/确认序号等控制信息
{
    static int seq_PKT=0;//使用静态局部变量
    char share_file_name[MAX_FILENANE_LEN];
    int share_file;
    sprintf(share_file_name, "%s%04d", DATALINK_NETWORK_SHARE_FILE, seq_PKT);
    inc_seq_PKT(seq_PKT);
    share_file = open(share_file_name, O_WRONLY | O_CREAT|O_TRUNC, 0644);

    write(share_file,p->data,MAX_PKT);
    close(share_file);
    //向NETWORK_LAYER发送enable 信号 的操作分离出去
    
}

void enable_network_layer_read(const char* proc_name)
{
    static pid=-1;
    while (pid==-1)//如果求过就不用再求了。
        pid=get_first_pid(proc_name);
    sendSIG(pid,MYSIG_DATALINK_LAYER_READY);
}
