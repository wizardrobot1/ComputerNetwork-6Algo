#include "../common/common.h"
#include "../common/tools.h"
#include "../common/d2n_layer.h"

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

    const char *network_proc = "network";
    signal(MYSIG_NETWORK_LAYER_READY, SIG_IGN); //����MYSIG_NETWORK_LAYER_READY�ź�
    printf("datalink ready \n");

    seq_nr next_frame_to_send;
    frame s;
    packet buffer;
    event_type event;

#ifndef MYDEBUG
    next_frame_to_send = 0;      //��ʼ֡��Ϊ0
    from_network_layer(&buffer); //ȡ��֡
    enable_network_layer(network_proc);//֪ͨ����㷢��һ����
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
                from_network_layer(&buffer);
                enable_network_layer(network_proc);//֪ͨ����㷢��һ����
                inc(next_frame_to_send);
            }
            //ACK������s.ack����
        }
        //���event=cksum_err
        //���event=timout
    } //end of while

#endif

#ifdef MYDEBUG

#endif
}