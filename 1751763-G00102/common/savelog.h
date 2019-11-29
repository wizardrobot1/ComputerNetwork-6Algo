#include "common.h"
#include "tools.h"

typedef enum
{
    rec_cksum_err,           
    rec_timeout,             
    rec_ack_timeout,          //ȷ�ϰ���ʱ
    rec_mismatch_data,             //��Ų�ƥ��
    rec_mismatch_ack,
    rec_outrange_ack,
    rec_repeat,             //���͹���ͬ��ŵİ���
    rec_nak,
    rec_cksum_nak
} record_reason;            //��¼�¼�����ö����

void flushlog();
void savelog(const char* record);
void record_err(seq_nr seq_expected,event_type event);
void record_repeat(int seq_start,int nrepeat,int seq_ack,event_type event);