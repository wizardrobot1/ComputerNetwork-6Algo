#include "../common/common.h"
#include "../common/tools.h"

#define MAX_SEQ 1
#define inc(k) if(k<MAX_SEQ) k=k+1; else k=0;

//#define MYDEBUG
static event_type catch_event;
static void SIGHANDLER_MYSIG_FRAME_ARRIVAL(int signo)
{
    catch_event = frame_arrival;
}
static void SIGHANDLER_MYSIG_CHSUM_ERR(int signo)
{
    catch_event = cksum_err;
}
static void wait_for_event(event_type *event) //�����������ȴ��¼�����
{
    signal(MYSIG_FRAME_ARRIVAL, SIGHANDLER_MYSIG_FRAME_ARRIVAL);
    signal(MYSIG_CHSUM_ERR, SIGHANDLER_MYSIG_CHSUM_ERR);
    pause();
    *event = catch_event;
}

int main()
{
    int pids[100];
    const char *network_proc = "receiver3_network";
    while (getpid_by_name(network_proc, pids) != 3)
    {
        sleep(1); //�ȴ�������
    }
    printf("receiver_datalink ready %d\n", pids[0]);

    seq_nr frame_expected;
    frame r, s;
    event_type event;
    
#ifndef MYDEBUG
    frame_expectd = 0;
    while (true)
    {
        wait_for_event(&event); //�������¼�
        if (event == frame_arrival)
        {
            from_physical_layer(&r);
            if (r.seq == frame_expected)
            { //���ƥ��
                to_network_layer(&r.info);
                inc(frame_expected);
            }
            s.ack = 1 - frame_expected; //�����Ƿ�ƥ��,0-1�任
            s.kind=ack;
            to_physical_layer(&s);
        }
        //event = cksum_err��������ֱ�ӵȴ���һ���¼�
    }
#endif

#ifdef MYDEBUG
    char share_file_name[256];
    int share_file, seq_PKT = 0;
    while (1)
    {
        sprintf(share_file_name, "%s%04d", NETWORK_DATALINK_SAHRE_FILE, seq_PKT); //�÷��ͷ�����㷢����·���PKT������
        inc_seq_PKT(seq_PKT);
        share_file = open(share_file_name, O_RDONLY);
        if (share_file == -1)
            break;
        read(share_file, r.info.data, MAX_PKT);
        to_network_layer(&r.info, pids[0]);
        close(share_file);
    }
#endif
}