#include "common.h"
#include "tools.h"

typedef enum
{
    rec_cksum_err,           
    rec_timeout,             
    rec_ack_timeout,          //确认包超时
    rec_mismatch_data,             //序号不匹配
    rec_mismatch_ack,
    rec_outrange_ack,
    rec_repeat,             //发送过相同序号的包了
    rec_nak,
    rec_cksum_nak
} record_reason;            //记录事件类型枚举量

void flushlog();
void savelog(const char* record);
void record_err(seq_nr seq_expected,event_type event);
void record_repeat(int seq_start,int nrepeat,int seq_ack,event_type event);