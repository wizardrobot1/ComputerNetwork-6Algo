#include"../common/common.h"
#include"../common/tools.h"


static int ena=1;

void SIGHANDLER_MYSIG_ENABLE_NETWORK_LAYER(int signo)
{
        signal(MYSIG_ENABLE_NETWORK_LAYER,SIGHANDLER_MYSIG_ENABLE_NETWORK_LAYER);
        ena=1;
}

void SIGHANDLER_MYSIG_DISABLE_NETWORK_LAYER(int signo)
{
        signal(MYSIG_DISABLE_NETWORK_LAYER,SIGHANDLER_MYSIG_DISABLE_NETWORK_LAYER);
        ena=0;
}

#define inc_seq_PKT(k) if(k<10000) k=k+1; else k=0;

int main()
{
    const char* share_file="network_datalink.share.";
    const char* datalink_proc="sender1_datalink";

    char filein[MAX_FILENANE_LEN],fileout[MAX_FILENANE_LEN];
    printf("������Ҫ������ļ���:\n");   
    scanf("%s",filein);

    FILE *fin=fopen(filein,"rb");  
    if (fin == NULL)
        error_exit("open file");
    
    
    char buffer[MAX_PKT+1];
    seq_nr seq_PKT=0;
    int rdsize;

    int pids[100];
    if (getpid_by_name(datalink_proc,pids)==0)//�����Ϻ�ʵ���Ϸ���ֵ��Ӧ����1
    {
        printf("������·������Ƿ���������\n");
        return 0;
    }

    signal(MYSIG_DISABLE_NETWORK_LAYER,SIGHANDLER_MYSIG_DISABLE_NETWORK_LAYER);
    signal(MYSIG_ENABLE_NETWORK_LAYER,SIGHANDLER_MYSIG_ENABLE_NETWORK_LAYER);

    int fdout,lock;
    while (!feof(fin))
    {
        
        if (ena&&(rdsize=fread(buffer,1,MAX_PKT,fin)))//���ena  &&  ���ܶ���������д
        {
            
            sprintf(fileout,"%s%04d",share_file,seq_PKT);
            fdout=open(fileout,O_WRONLY|O_CREAT,0644);
            if (fdout == -1)
                error_exit("open file");
            
            write(fdout,buffer,rdsize);
            
            inc_seq_PKT(seq_PKT);
            kill(pids[0],MYSIG_NETWORK_LAYER_READY);
        }
        
        pause();
        
        if (ena)//datalink�ɹ�����share�ļ���,�������ر��ļ�����ʼд��һ���ļ�
        {
            set_lock(fdout,F_UNLCK);
            lock=0;
            close(fdout);
        }  
        else//datalink �� �������ˣ�û�ն�share�ļ������������Ҳ���д��һ���ļ�
        {
            if (!lock)//��ֹ�ظ���ͬһ���ļ���������
            {
                set_lock(fdout,F_WRLCK);  
                lock=1;
            }
        }        
        
        

    }
    fclose(fin);
    
}