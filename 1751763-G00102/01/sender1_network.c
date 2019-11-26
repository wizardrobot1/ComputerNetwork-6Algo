#include "../common/common.h"
#include "../common/tools.h"

static int ena = 1;

static void SIGHANDLER_MYSIG_ENABLE_NETWORK_LAYER(int signo)
{
    signal(MYSIG_ENABLE_NETWORK_LAYER, SIGHANDLER_MYSIG_ENABLE_NETWORK_LAYER);
    ena = 1;
}

static void SIGHANDLER_MYSIG_DISABLE_NETWORK_LAYER(int signo)
{
    signal(MYSIG_DISABLE_NETWORK_LAYER, SIGHANDLER_MYSIG_DISABLE_NETWORK_LAYER);
    ena = 0;
}



int main()
{
    const char *share_file = NETWORK_DATALINK_SAHRE_FILE;
    const char *datalink_proc = "sender1_datalink";

    char filein[MAX_FILENANE_LEN], fileout[MAX_FILENANE_LEN];
    printf("请输入要传输的文件名:\n");
    scanf("%s", filein);

    FILE *fin = fopen(filein, "rb");
    if (fin == NULL)
        error_exit("open file");

    char buffer[MAX_PKT + 1];
    seq_nr seq_PKT = 0;
    int rdsize=-1, i;

    int pids[100];
    if (getpid_by_name(datalink_proc, pids) == 0) //理论上和实际上返回值都应该是1
    {
        printf("请检测链路层程序是否正在运行\n");
        return 0;
    }

    signal(MYSIG_DISABLE_NETWORK_LAYER, SIGHANDLER_MYSIG_DISABLE_NETWORK_LAYER);
    signal(MYSIG_ENABLE_NETWORK_LAYER, SIGHANDLER_MYSIG_ENABLE_NETWORK_LAYER);

    int fdout,lock=0;
    while (rdsize)
    {

        if (ena) //如果ena,写入共享文件，发送ready信号
        {
            rdsize = fread(buffer, 1, MAX_PKT, fin);
            if (rdsize % MAX_PKT || !rdsize) //倒数第二个包（文件数据的最后一个包）不是1024的整数倍，补尾0;最后一次个包全0
                for (i = rdsize; i < MAX_PKT; i++)
                    buffer[i] = '\0';

            sprintf(fileout, "%s%04d", share_file, seq_PKT);
            fdout = open(fileout, O_WRONLY | O_CREAT, 0644);
            if (fdout == -1)
                error_exit("open file");

            write(fdout, buffer, MAX_PKT);

            inc_seq_PKT(seq_PKT);

            sendSIG(pids[0],MYSIG_NETWORK_LAYER_READY);
        }

        pause();

        if (ena) //datalink成功读完share文件了,开锁，关闭文件，开始写下一个文件
        {
            if (lock)
            {
                set_lock(fdout, F_UNLCK);
                lock = 0;
            }
            close(fdout);
        }
        else //datalink 的 窗口满了，没空读share文件，加锁，并且不能写下一个文件
        {
            if (!lock) //防止重复锁同一个文件导致阻塞
            {
                set_lock(fdout, F_WRLCK);
                lock = 1;
            }
        }
    }

    fclose(fin);
}