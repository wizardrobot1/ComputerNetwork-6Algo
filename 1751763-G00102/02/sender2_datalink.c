#include "../common/common.h"
#include "../common/tools.h"
#include "../common/d2n_layer.h"
#include "../common/p2d_layer.h"

static event_queue eventqueue= NULL;

static void SIGHANDLER_MYSIG_FRAME_ARRIVAL(int signo)
{
    event_queue p=eventqueue;
    while(p&&p->next)
    {
        p=p->next;
    }
    if(!p)
    {
        eventqueue=(event_queue)malloc(sizeof(struct event_queue_node));
        p=eventqueue;
    }
    else
    {
        p->next=(event_queue)malloc(sizeof(struct event_queue_node));
        p=p->next;
    }
    p->event = frame_arrival;
    p->next = NULL;
    //kill(getpid(),MYSIG_CONTINUE);
}


static void wait_for_event(event_type *event) //阻塞函数，等待事件发生
{
    signal(MYSIG_FRAME_ARRIVAL, SIGHANDLER_MYSIG_FRAME_ARRIVAL);
    if(!eventqueue)
        pause();

    event_queue p=eventqueue;    
    *event = eventqueue->event;
    eventqueue=eventqueue->next;
    free(p);
}

int main()
{
#ifndef MYDEBUG
    const char *physical_proc = "physical";
    const char *network_proc = "network";
	if (get_first_pid(physical_proc)==-1)//因为要向物理层发信号提醒它发文件，所以要先打开物理层
    {
        printf("plz start physical_layer first");
        return 0;
    }   


	printf("datalink ready \n");

    frame s;
    packet buffer;
    event_type event;
    while (1)
    {	
        from_network_layer(&buffer);
        enable_network_layer(network_proc);//通知网络层发下一数据
        memcpy(s.info.data, buffer.data, MAX_PKT);
        s.kind=data;
        to_physical_layer(&s);
 		enable_physical_layer(PHYSICAL);
        wait_for_event(&event);
    }



#endif

#ifdef MYDEBUG
    const char *network_proc = "network";
    if (get_first_pid("network")==-1)
	{
        printf("plz start network_layer first");
        return 0;
    }
    frame s;
    packet buffer;
    int ftest = open("test1", O_WRONLY | O_CREAT, 0644);
    while (1)
    {
        from_network_layer(&buffer);
        enable_network_layer(network_proc);//通知网络层发下一数据
        memcpy(s.info.data, buffer.data, 1024);
        write(ftest, s.info.data, MAX_PKT);

        //to_physical_layer(&s);
    }

    close(ftest);
#endif

}