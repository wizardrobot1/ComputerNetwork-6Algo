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
static void SIGHANDLER_MYSIG_TIMEOUT(int signo)
{
    catch_event = timeout;
}
static void SIGHANDLER_MYSIG_CHSUM_ERR(int signo)
{
    catch_event = cksum_err;
}
static void wait_for_event(event_type *event) //�����������ȴ��¼�����
{
    signal(MYSIG_FRAME_ARRIVAL, SIGHANDLER_MYSIG_FRAME_ARRIVAL);
    signal(MYSIG_TIMEOUT, SIGHANDLER_MYSIG_TIMEOUT);
    signal(MYSIG_CHSUM_ERR, SIGHANDLER_MYSIG_CHSUM_ERR);
    pause();
    *event = catch_event;
}

int main()
{

    int pids[100];
    const char *network_proc = "sender3_network";
    signal(MYSIG_NETWORK_LAYER_READY, SIG_IGN);     //����MYSIG_NETWORK_LAYER_READY�ź�
    while (getpid_by_name(network_proc, pids) != 3) //һ��sh , һ�� grep , һ�� ./sender1_network
    {
        sleep(1); //�ȴ�������
    }

    seq_nr next_frame_to_send;
    frame s;
    packet buffer;
    event_type event;

#ifndef MYDEBUG
    next_frame_to_send = 0;      //��ʼ֡��Ϊ0
    from_network_layer(&buffer,pid[0]); //ȡ��֡
    while (1)
    {
        s.info = buffer;
        s.seq = next_frame_to_send;
        s.kind=data;
        to_physical_layer(&s);
        start_timer(s.seq);
        wait_for_event(&event); //�������¼�
        if (event == frame_arrival)
        {
            from_physical_layer(&s);
            if (s.ack == next_frame_to_send)
            {
                stop_timer(s.ack);
                from_network_layer(&buffer,pid[0]);
                inc(next_frame_to_send);
            }
            //ACK������s.ack����
        }
        //���event=cksum_err
        //���event=timout
    } //end of while

#endif

#ifdef MYDEBUG

    printf("receiver_datalink ready %d\n", pids[0]);

    int ftest = open("test1", O_WRONLY | O_CREAT, 0644);
    while (1)
    {
        from_network_layer(&buffer, pids[0]);
        memcpy(s.info.data, buffer.data, 1024);
        write(ftest, s.info.data, MAX_PKT);

        //to_physical_layer(&s);
    }

    close(ftest);
#endif
}