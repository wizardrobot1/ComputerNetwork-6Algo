#include "../common/common.h"
#include "../common/tools.h"

static int ena = 0;

static void SIGHANDLER_MYSIG_DATALINK_LAYER_READY(int signo)
{
    signal(MYSIG_DATALINK_LAYER_READY, SIGHANDLER_MYSIG_DATALINK_LAYER_READY);
    ena = 1;
}

#define inc_seq_PKT(k) \
    if (k < 10000)     \
        k = k + 1;     \
    else               \
        k = 0;

int main()
{
    const char *share_file = "datalink_network.share."; //����network_datalink

    const char*fileout = "���յ����ļ�";
    int fdout = open(fileout, O_WRONLY | O_CREAT, 0644);
    if (fdout == -1)
        error_exit("open file");
    char filein[MAX_FILENANE_LEN];

    FILE *fin;
    if (fin == NULL)
        error_exit("open file");

    char buffer[2][MAX_PKT + 1];//��ΪҪ�жϽ������󣬲Ż�Ե����������д���
    memset(buffer[0], 0, MAX_PKT + 1);
    memset(buffer[1], 0, MAX_PKT + 1);

    seq_nr seq_PKT = 0;//�����������Ǹ���0000~9999
    int seq_buf = 0;//���������ĸ�����������0~1


    signal(MYSIG_DATALINK_LAYER_READY, SIGHANDLER_MYSIG_DATALINK_LAYER_READY); //RNL �յ�����źź���ļ�

    int i, end_of_file = 0, rdsize = 0, last_buf, last_PKT_size,first_PKT=1;
    while (1)
    {

        if (ena) //���ena
        {
            last_buf = seq_buf;
            seq_buf=1-seq_buf;
            //����·��ȡ����
            sprintf(filein, "%s%04d", share_file, seq_PKT);
            inc_seq_PKT(seq_PKT);
            fin = fopen(filein, "rb");
            if (fin==NULL)
                error_exit("open file");
            rdsize = fread(buffer[seq_buf], 1, MAX_PKT, fin);
            fclose(fin);
            
            //�ж��ǲ��ǽ�����
            end_of_file = 1;
            for (i = 0; i < MAX_PKT; i++)
                if (buffer[seq_buf][i] != '\0') //�������ȫ0������û�ꡣ
                {
                    end_of_file = 0;
                    break;
                }
            
            //����ǽ�������������˳�ѭ��
            if (end_of_file) //�յ�������,buffer[last_buffer]�����һ�������ݵİ�,д����˳�whileѭ��
            {
                
                last_PKT_size = 0;
                for (i = MAX_PKT - 1; i >= 0; i--) //����һ���������һ������Ԫ��λ��,���û���ҵ�����һ�����Ĵ�С����0����Ȼ�����ܷ������������
                    if (buffer[last_buf][i] != '\0')
                    {
                        last_PKT_size = i + 1;
                        break;
                    }
                write(fdout, buffer[last_buf], last_PKT_size);
                break;
            }
            //���ǽ����������԰���һ������ȫд������ļ���
            if (!first_PKT)//������ǵ�һ����������һ����д���ļ�(�ض���MAX_PKT��С)
                write(fdout, buffer[last_buf],MAX_PKT);
            else    
                first_PKT=0;

        }

        pause();
    }
    close(fdout);
}