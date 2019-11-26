#include"../common/common.h"
#include"../common/tools.h"


static event_type catch_event;
static void SIGHANDLER_MYSIG_FRAME_ARRIVAL(int signo)
{
    catch_event=frame_arrival;
}
static void wait_for_event(event_type *event)//�����������ȴ��¼�����
{
    signal(MYSIG_FRAME_ARRIVAL, SIGHANDLER_MYSIG_FRAME_ARRIVAL);
    pause();
    *event=catch_event;
}

int main()
{
    frame r;
    event_type event;

    int pids[100];
    const char *network_proc = "receiver1_network";
    while (getpid_by_name(network_proc, pids) == 0) //�����Ϻ�ʵ���Ϸ���ֵ��Ӧ����1
    {
        printf("not open\n");
        sleep(1);//�ȴ�������
    }
    printf("receiver_datalink ready\n");

    char share_file_name[256],buffer[1024];
    int share_file,seq_PKT=0;
    while(1)
    {
        wait_for_event(&event);
        from_physical_layer(&r);
        to_network_layer(&r.info,pids[0]);
    }
}