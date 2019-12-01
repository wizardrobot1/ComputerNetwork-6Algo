#include "../common/common.h"
#include "../common/tools.h"
#include "../common/p2d_layer.h"


int main(int argc,char* argv[])
{
    if (argc<3)
    {
        printf("run program + Ŀ��ip + Ŀ��˿�");
        return 0;
    }
    if (get_first_pid(DATALINK)==-1)//��ΪҪ����·�㷢�ź����������ļ�������Ҫ�ȴ���·
    {
        printf("plz start datalink_layer first");
        return 0;
    }
    printf("physical ready \n");
    signal(MYSIG_DISABLE_PHYSICAL_LAYER, SIG_IGN);

    frame r;
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
        
    }

	printf("RPL rece finished.\n");

	close(listenfd);close(connfd);
}