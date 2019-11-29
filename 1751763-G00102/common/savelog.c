#include "savelog.h"


void flushlog()
{
    FILE* flog=fopen(LOG_PATH,"w+");
    fclose(flog);
}
void savelog(const char* record)
{
    char now_time[100];
    int pid=getpid();
    get_datatime(now_time);
    FILE* flog=fopen(LOG_PATH,"a+");
    if (flog==NULL)
        error_exit("open log file");
    fprintf(flog,"��%s������%d��¼�¼�:%s\n",now_time,pid,record);
    fclose(flog);
}

/*
�����cksum_err��seq_expected=frame_expected
�����timeout��seq_expected=ack_expected
�����ack_timeout,seq_expected=(frame_expected + MAX_SEQ) % (MAX_SEQ + 1)
*/
void record_err(seq_nr seq_expected,event_type event)
{
    char err_detail[100];
    switch (event)
    {
        case rec_cksum_err:
            sprintf(err_detail,"�ȴ���%d�����ݰ�/ack��ʱ����У�����",seq_expected);
            break;
        case rec_timeout:
            sprintf(err_detail,"��%d�����ݰ���ٳ�δ�յ�ack����ʱ",seq_expected);
            break;
        case rec_ack_timeout:
            sprintf(err_detail,"��%d��ack����ٳ�û��������������ʱ",seq_expected);
            break;
        case rec_mismatch_data:
            sprintf(err_detail,"��%d�����ݰ����յ������ݰ���Ų�ƥ��",seq_expected);
            break;
        case rec_mismatch_ack:
            sprintf(err_detail,"��%d��ack�����յ���ack����Ų�ƥ��",seq_expected);
            break;
        case rec_outrange_ack:
            sprintf(err_detail,"�ӵ�%d������ʼ�ķ��ʹ�����û�����յ���ack�����ƥ������",seq_expected);
            break;
        case rec_repeat:
            sprintf(err_detail,"��%d�����ݰ��ظ��յ�",seq_expected);
            break;
        case rec_nak:
            sprintf(err_detail,"��%d�����ݰ����յ������ݰ���Ų�ƥ��,�����ɻ��ɴ�������ˣ����Է�nak��",seq_expected);
            break;
        case rec_cksum_nak:
            sprintf(err_detail,"�ȴ���%d�����ݰ����յ�У�����,�����ɻ��ɴ�������ˣ����Է�nak��",seq_expected);
            break;    
        default:
            sprintf(err_detail,"��%d����Ҳ��֪��ô�ĳ�����",seq_expected);
            break;      
    }
    savelog(err_detail);
}

/*
*/
void record_repeat(int seq_start,int nrepeat,int seq_ack,event_type event)
{
    char repeat_detail[100];
    switch (event)
    {
        case rec_cksum_err:
            if (seq_start==-1)
                sprintf(repeat_detail,"����У������ش���%d��ack��",seq_ack);
            else if (seq_ack==-1)
                sprintf(repeat_detail,"����У������ش��ӵ�%d��ʼ��%d�����ݰ�",seq_start,nrepeat);
            else
                sprintf(repeat_detail,"����У������ش��ӵ�%d��ʼ��%d�����ݰ��Ӵ���%d��ack��",seq_start,nrepeat,seq_ack);
            break;
        case rec_timeout:
            if (seq_start==-1)
                sprintf(repeat_detail,"���ڳٳ�δ�յ�ack���ش���%d��ack��",seq_ack);
            else if (seq_ack==-1)
                sprintf(repeat_detail,"���ڳٳ�δ�յ�ack���ش��ӵ�%d��ʼ��%d�����ݰ�",seq_start,nrepeat);
            else
                sprintf(repeat_detail,"���ڳٳ�δ�յ�ack���ش��ӵ�%d��ʼ��%d�����ݰ��Ӵ���%d��ack��",seq_start,nrepeat,seq_ack);
            break;
        case rec_ack_timeout:
            sprintf(repeat_detail,"���ڳٳ�û������������ֻ�ܵ��������%d��ack��",seq_ack);
            break;
        case rec_mismatch_data:
            sprintf(repeat_detail,"�����յ������ݰ���Ų�ƥ��,�ش���%d��ack��",seq_ack);
            break;
        case rec_mismatch_ack:
            sprintf(repeat_detail,"�����յ���ack����Ų�ƥ��,�ش���%d�����ݰ�",seq_start);
            break;
        case rec_nak:
            sprintf(repeat_detail,"�����յ�nak��,�ش���%d�����ݰ�",seq_start);
            break;
        default:
            sprintf(repeat_detail,"Ҳ��֪��ô�ģ��ش��ӵ�%d�����ݰ���ʼ��%d�����ݰ�",seq_start,nrepeat);
            break;      
    }
    savelog(repeat_detail);
}
