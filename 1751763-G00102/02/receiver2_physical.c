#include "../common/common.h"
#include "../common/tools.h"
#include "../common/p2d_layer.h"

static int phy_ena=0;

static void SIGHANDLER_MYSIG_ENABLE_PHYSICAL_LAYER(int signo)
{
    signal(MYSIG_ENABLE_PHYSICAL_LAYER, SIGHANDLER_MYSIG_ENABLE_PHYSICAL_LAYER);
    phy_ena ++;
}


int main(int argc,char* argv[])
{
    if (argc<3)
    {
        printf("run program + 目标ip + 目标端口");
        return 0;
    }
    if (get_first_pid(DATALINK)==-1)//因为要向链路层发信号提醒它读文件，所以要先打开链路
    {
        printf("plz start datalink_layer first");
        return 0;
    }
    printf("physical ready \n");
    
    signal(MYSIG_ENABLE_PHYSICAL_LAYER, SIGHANDLER_MYSIG_ENABLE_PHYSICAL_LAYER);
    frame r,s;
    event_type event;    

	int listenfd,connfd;
	connFDr(&listenfd,&connfd,argv[1],argv[2]);
	printf("RPL connects SPL.\n");

    int n;
    while (1)
    {
		if((n=rece_phy(&r,connfd))<=0)
			break;
        to_datalink_layer(&r);
        enable_datalink_layer_read(DATALINK);
        while (!phy_ena)
            ;
        srand(time(0));
        if (rand()%10==0)
            sleep(1);
        from_datalink_layer(&s);
		send_phy(&s,connfd);
        phy_ena--;
    }

	printf("RPL rece finished.\n");

	close(listenfd);close(connfd);
}