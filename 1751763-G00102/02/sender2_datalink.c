#include "../common/common.h"
#include "../common/tools.h"

//#define MYDEBUG

static event_type catch_event;
static void SIGHANDLER_MYSIG_FRAME_ARRIVAL(int signo)
{
    catch_event = frame_arrival;
}
static void wait_for_event(event_type *event) //�����������ȴ��¼�����
{
    signal(MYSIG_FRAME_ARRIVAL, SIGHANDLER_MYSIG_FRAME_ARRIVAL);
    pause();
    *event = catch_event;
}


int main()
{
    
    int pids[100];
    const char *network_proc = "sender2_network";
    signal(MYSIG_NETWORK_LAYER_READY, SIG_IGN); //����MYSIG_NETWORK_LAYER_READY�ź�

    frame s;
    packet buffer;
    event_type event;
    
#ifndef MYDEBUG

    while (getpid_by_name(network_proc, pids) != 3) //һ��sh , һ�� grep , һ�� ./sender1_network
    {
        sleep(1); //�ȴ�������
    }

    while (1)
    {
        from_network_layer(&buffer, pids[0]);
        memcpy(s.info.data, buffer.data, 1024);
        s.kind=data;
        //to_physical_layer(&s);
        wait_for_event(&event);
    }
#endif

#ifdef MYDEBUG
    while (getpid_by_name(network_proc, pids) != 3) //һ��sh , һ�� grep , һ�� ./sender1_network
    {
        sleep(1); //�ȴ�������
    }
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