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
    fprintf(flog,"在%s，进程%d记录事件:%s\n",now_time,pid,record);
    fclose(flog);
}

/*
如果是cksum_err，seq_expected=frame_expected
如果是timeout，seq_expected=ack_expected
如果是ack_timeout,seq_expected=(frame_expected + MAX_SEQ) % (MAX_SEQ + 1)
*/
void record_err(seq_nr seq_expected,event_type event)
{
    char err_detail[100];
    switch (event)
    {
        case rec_cksum_err:
            sprintf(err_detail,"等待第%d个数据包/ack包时遇到校验错误",seq_expected);
            break;
        case rec_timeout:
            sprintf(err_detail,"第%d个数据包因迟迟未收到ack而超时",seq_expected);
            break;
        case rec_ack_timeout:
            sprintf(err_detail,"第%d个ack包因迟迟没有逆向流量而超时",seq_expected);
            break;
        case rec_mismatch_data:
            sprintf(err_detail,"第%d个数据包与收到的数据包序号不匹配",seq_expected);
            break;
        case rec_mismatch_ack:
            sprintf(err_detail,"第%d个ack包与收到的ack包序号不匹配",seq_expected);
            break;
        case rec_outrange_ack:
            sprintf(err_detail,"从第%d个包开始的发送窗口中没有与收到的ack包序号匹配的序号",seq_expected);
            break;
        case rec_repeat:
            sprintf(err_detail,"第%d个数据包重复收到",seq_expected);
            break;
        case rec_nak:
            sprintf(err_detail,"第%d个数据包与收到的数据包序号不匹配,有理由怀疑传输出错了，所以发nak包",seq_expected);
            break;
        case rec_cksum_nak:
            sprintf(err_detail,"等待第%d个数据包是收到校验错误,有理由怀疑传输出错了，所以发nak包",seq_expected);
            break;    
        default:
            sprintf(err_detail,"第%d个包也不知怎么的出错了",seq_expected);
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
                sprintf(repeat_detail,"由于校验错误，重传第%d个ack包",seq_ack);
            else if (seq_ack==-1)
                sprintf(repeat_detail,"由于校验错误，重传从第%d开始的%d个数据包",seq_start,nrepeat);
            else
                sprintf(repeat_detail,"由于校验错误，重传从第%d开始的%d个数据包捎带第%d个ack包",seq_start,nrepeat,seq_ack);
            break;
        case rec_timeout:
            if (seq_start==-1)
                sprintf(repeat_detail,"由于迟迟未收到ack，重传第%d个ack包",seq_ack);
            else if (seq_ack==-1)
                sprintf(repeat_detail,"由于迟迟未收到ack，重传从第%d开始的%d个数据包",seq_start,nrepeat);
            else
                sprintf(repeat_detail,"由于迟迟未收到ack，重传从第%d开始的%d个数据包捎带第%d个ack包",seq_start,nrepeat,seq_ack);
            break;
        case rec_ack_timeout:
            sprintf(repeat_detail,"由于迟迟没有逆向流量，只能单独传输第%d个ack包",seq_ack);
            break;
        case rec_mismatch_data:
            sprintf(repeat_detail,"由于收到的数据包序号不匹配,重传第%d个ack包",seq_ack);
            break;
        case rec_mismatch_ack:
            sprintf(repeat_detail,"由于收到的ack包序号不匹配,重传第%d个数据包",seq_start);
            break;
        case rec_nak:
            sprintf(repeat_detail,"由于收到nak包,重传第%d个数据包",seq_start);
            break;
        default:
            sprintf(repeat_detail,"也不知怎么的，重传从第%d个数据包开始的%d个数据包",seq_start,nrepeat);
            break;      
    }
    savelog(repeat_detail);
}
