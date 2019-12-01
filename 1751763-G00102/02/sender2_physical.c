#include "../common/common.h"
#include "../common/tools.h"
#include "../common/p2d_layer.h"

static int phy_ena=0;

static void SIGHANDLER_MYSIG_ENABLE_PHYSICAL_LAYER(int signo)
{
    signal(MYSIG_ENABLE_PHYSICAL_LAYER, SIGHANDLER_MYSIG_ENABLE_PHYSICAL_LAYER);
    phy_ena ++;
}

//#define MYDEBUG

int main(int argc,char* argv[])
{
    if (argc<3)
    {
        printf("run program + 目标ip + 目标端口");
        return 0;
    }
    signal(MYSIG_ENABLE_PHYSICAL_LAYER, SIGHANDLER_MYSIG_ENABLE_PHYSICAL_LAYER);


    printf("physical ready \n");

    frame s,r;
    packet buffer;

    int sockfd;
	connFDs(&sockfd,argv[1],argv[2]);
	printf("SPL connects RPL.\n");
	int n;
    while (1)
    {
		if(phy_ena<=0)continue;	    
	    from_datalink_layer(&s);
		send_phy(&s,sockfd);
        phy_ena--;

        if((n=rece_phy(&r,sockfd))<=0)
			break;
        to_datalink_layer(&r);
        enable_datalink_layer_read(DATALINK);
    }

	printf("SPL sends finished.\n");
	close(sockfd);
	
}

