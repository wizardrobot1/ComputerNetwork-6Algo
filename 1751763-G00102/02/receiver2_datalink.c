#include "../common/common.h"
#include "../common/tools.h"
#include "../common/d2n_layer.h"

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

    const char *network_proc = "network";

    if (get_first_pid(network_proc)==-1)//��ΪҪ������㷢�ź����������ļ�������Ҫ�ȴ������
    {
        printf("plz start netwrok_layer first");
        return 0;
    }
    printf("datalink ready \n");

    frame r,s;
    event_type event;
#ifndef MYDEBUG
    while (1)
    {
        wait_for_event(&event);
        from_physical_layer(&r);
        to_network_layer(&r.info);
        enable_network_layer_read(network_proc);
        s.kind=ack;
        s.seq=0xffffffff;
        to_physical_layer(&s);
    }
#endif

#ifdef MYDEBUG

#endif
}