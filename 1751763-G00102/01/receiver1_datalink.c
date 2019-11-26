#include"../common/common.h"
#include"../common/tools.h"


static event_type catch_event;
static void SIGHANDLER_MYSIG_FRAME_ARRIVAL(int signo)
{
    catch_event=frame_arrival;
}
static void wait_for_event(event_type *event)//阻塞函数，等待事件发生
{
    signal(MYSIG_FRAME_ARRIVAL, SIGHANDLER_MYSIG_FRAME_ARRIVAL);
    pause();
    *event=catch_event;
}

int main()
{
    frame r;
    event_type event;
    while(1)
    {
        wait_for_event(&event);
        from_physical_layer(&r);    
        to_network_layer(&r.info);
    }
}