#include"common.h"
#include"tools.h"
void from_network_layer(packet *p,int pid)//���ͷ��������õ������ݰ�
{
    static int seq_PKT=0;//ʹ�þ�̬�ֲ�����
    char share_file_name[MAX_FILENANE_LEN];
    int share_file;
    sprintf(share_file_name, "%s%04d", NETWORK_DATALINK_SAHRE_FILE, seq_PKT);
    inc_seq_PKT(seq_PKT);
    share_file = open(share_file_name, O_RDONLY);

    //��������ȡ�ļ�
    set_lock(share_file,F_WRLCK);
    read(share_file,p->data,MAX_PKT);
    set_lock(share_file,F_UNLCK);//���꿪��
    //��NETWORK_LAYER����enable �ź�
    kill(pid,MYSIG_ENABLE_NETWORK_LAYER);
}

void to_network_layer(packet *p,int pid)//���շ�������㷢�ʹ����ݰ�,ȥ��֡�����͡�����/ȷ����ŵȿ�����Ϣ
{
    static int seq_PKT=0;//ʹ�þ�̬�ֲ�����
    char share_file_name[MAX_FILENANE_LEN];
    int share_file;
    sprintf(share_file_name, "%s%04d", DATALINK_NETWORK_SAHRE_FILE, seq_PKT);
    inc_seq_PKT(seq_PKT);
    share_file = open(share_file_name, O_WRONLY | O_CREAT, 0644);

    write(share_file,p->data,MAX_PKT);

    //��NETWORK_LAYER����enable �ź�
    kill(pid,MYSIG_DATALINK_LAYER_READY);
}
