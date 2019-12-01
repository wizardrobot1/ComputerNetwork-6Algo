#include "../common/common.h"
#include "../common/tools.h"
#include "../common/d2n_layer.h"
#include "../common/p2d_layer.h"



int main()
{
#ifndef MYDEBUG
    const char *physical_proc = "physical";
    const char *network_proc = "network";
	if (get_first_pid(physical_proc)==-1)//��ΪҪ������㷢�ź����������ļ�������Ҫ�ȴ������
    {
        printf("plz start physical_layer first");
        return 0;
    }   


	printf("datalink ready \n");

    frame s;
    packet buffer;
    
    while (1)
    {	
        from_network_layer(&buffer);
        enable_network_layer(network_proc);//֪ͨ����㷢��һ����
        memcpy(s.info.data, buffer.data, MAX_PKT);
        s.kind=data;
        to_physical_layer(&s);
 		enable_physical_layer(PHYSICAL);

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
        enable_network_layer(network_proc);//֪ͨ����㷢��һ����
        memcpy(s.info.data, buffer.data, 1024);
        write(ftest, s.info.data, MAX_PKT);

        //to_physical_layer(&s);
    }

    close(ftest);
#endif

}