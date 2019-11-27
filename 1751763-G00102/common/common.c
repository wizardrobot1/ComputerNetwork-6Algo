#include"common.h"
#include"tools.h"


static frame_timer *mytimer = NULL;


void from_network_layer(packet *p,int pid)//���ͷ��������õ������ݰ�
{
    static int seq_PKT=0;//ʹ�þ�̬�ֲ�����
    char share_file_name[MAX_FILENANE_LEN];
    int share_file=-1;
    sprintf(share_file_name, "%s%04d", NETWORK_DATALINK_SAHRE_FILE, seq_PKT);
    inc_seq_PKT(seq_PKT);
    while (share_file==-1)
    {
        share_file = open(share_file_name, O_RDONLY);
    }
    //��������ȡ�ļ�
    //printf("set lock to %s %d\n",share_file_name,share_file);
    set_lock(share_file,F_RDLCK);
    read(share_file,p->data,MAX_PKT);
    set_lock(share_file,F_UNLCK);//���꿪��
    close(share_file);
    //��NETWORK_LAYER����enable �ź�
    sendSIG(pid,MYSIG_ENABLE_NETWORK_LAYER);
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
    close(share_file);
    //��NETWORK_LAYER����enable �ź�
    kill(pid,MYSIG_DATALINK_LAYER_READY);
}

void from_physical_layer(frame *f){};//���շ��������ȡ��֡,֡ͷβ��FLAG�ֽڡ������е��ֽ�������ȥ��,���ñ�����ǰ����֤��У��ͣ���������������cksum_err�¼������ֻ��֡��ȷ������»���ñ�����

void to_physical_layer(frame *f){};//���ͷ�������㷢��֡,֡ͷβ��FLAG�ֽڡ������н����ֽ����,����У��ͷ���֡β

void start_timer(seq_nr k)
{
    static frame_timer *rear = mytimer;
    frame_timer *p=mytimer;

    switch k :
    {
        case 0:
        {
            myhead=(frame_second)malloc(sizeof(frame_second));
            myhead->sec=MYTIMER_TIMEOUT_TIME;
        }
        default :
        {

        }
    }

};//������k֡�Ķ�ʱ��

void stop_timer(seq_nr k)
{

};//ֹͣ��k֡�Ķ�ʱ��

void start_ack_timer(void){};//����ȷ�ϰ���ʱ��

void stop_ack_timer(void){};//ֹͣȷ�ϰ���ʱ��

void enable_network_layer(void){};//������������,ʹ���Բ����µ�network_layer_ready�¼�


void disable_network_layer(void){};//ʹ���������,���ٲ����µ�network_layer_ready�¼�