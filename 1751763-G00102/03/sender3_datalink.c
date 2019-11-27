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
static void wait_for_event(event_type *event) //阻塞函数，等待事件发生
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
    signal(MYSIG_NETWORK_LAYER_READY, SIG_IGN);     //屏蔽MYSIG_NETWORK_LAYER_READY信号
    while (getpid_by_name(network_proc, pids) != 3) //一个sh , 一个 grep , 一个 ./sender1_network
    {
        sleep(1); //等待网络层打开
    }

    seq_nr next_frame_to_send;
    frame s;
    packet buffer;
    event_type event;

#ifndef MYDEBUG
    next_frame_to_send = 0;      //初始帧号为0
    from_network_layer(&buffer,pid[0]); //取首帧
    while (1)
    {
        s.info = buffer;
        s.seq = next_frame_to_send;
        s.kind=data;
        to_physical_layer(&s);
        start_timer(s.seq);
        wait_for_event(&event); //等三个事件
        if (event == frame_arrival)
        {
            from_physical_layer(&s);
            if (s.ack == next_frame_to_send)
            {
                stop_timer(s.ack);
                from_network_layer(&buffer,pid[0]);
                inc(next_frame_to_send);
            }
            //ACK到，但s.ack不对
        }
        //如果event=cksum_err
        //如果event=timout
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