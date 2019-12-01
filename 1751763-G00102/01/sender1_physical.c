#include "../common/common.h"
#include "../common/tools.h"
#include "../common/p2d_layer.h"

static int phy_ena=0;

static void SIGHANDLER_MYSIG_ENABLE_PHYSICAL_LAYER(int signo)
{
    signal(MYSIG_ENABLE_PHYSICAL_LAYER, SIGHANDLER_MYSIG_ENABLE_PHYSICAL_LAYER);
    phy_ena ++;
}

static int phy_cir=1;
static void SIGHANDLER_MYSIG_DISABLE_PHYSICAL_LAYER(int signo)
{
    signal(MYSIG_DISABLE_PHYSICAL_LAYER, SIGHANDLER_MYSIG_DISABLE_PHYSICAL_LAYER);
    phy_cir = 0;
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

    frame s;
    packet buffer;

    int sockfd;
	connFDs(&sockfd,argv[1],argv[2]);
	printf("SPL connects RPL.\n");
	
    while (phy_cir||phy_ena>0)
    {
		if(phy_ena<=0)continue;	    
	    from_datalink_layer(&s);
		send_phy(&s,sockfd);
        phy_ena--;
    }

	printf("SPL sends finished.\n");
	close(sockfd);
	
}

