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
    signal(MYSIG_NETWORK_LAYER_READY, SIG_IGN); //����MYSIG_NETWORK_LAYER_READY�ź�
    printf("datalink ready \n");

    frame s;
    packet buffer;
    event_type event;
    
#ifndef MYDEBUG

    while (1)
    {
        from_network_layer(&buffer);
        enable_network_layer(network_proc);//֪ͨ����㷢��һ����
        memcpy(s.info.data, buffer.data, 1024);
        s.kind=data;
        to_physical_layer(&s);
        wait_for_event(&event);
    }
#endif

#ifdef MYDEBUG

#endif
}