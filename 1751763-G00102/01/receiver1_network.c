#include "../common/common.h"
#include "../common/tools.h"

static int ena = 0;

static void SIGHANDLER_MYSIG_DATALINK_LAYER_READY(int signo)
{
    signal(MYSIG_DATALINK_LAYER_READY, SIGHANDLER_MYSIG_DATALINK_LAYER_READY);
    ena = 1;
}

#define inc_seq_PKT(k) \
    if (k < 10000)     \
        k = k + 1;     \
    else               \
        k = 0;

int main()
{
    const char *share_file = "datalink_network.share."; //不是network_datalink

    const char*fileout = "接收到的文件";
    int fdout = open(fileout, O_WRONLY | O_CREAT, 0644);
    if (fdout == -1)
        error_exit("open file");
    char filein[MAX_FILENANE_LEN];

    FILE *fin;
    if (fin == NULL)
        error_exit("open file");

    char buffer[2][MAX_PKT + 1];//因为要判断结束包后，才会对倒二个包进行处理
    memset(buffer[0], 0, MAX_PKT + 1);
    memset(buffer[1], 0, MAX_PKT + 1);

    seq_nr seq_PKT = 0;//接下来接收那个包0000~9999
    int seq_buf = 0;//接下来用哪个缓存区接收0~1


    signal(MYSIG_DATALINK_LAYER_READY, SIGHANDLER_MYSIG_DATALINK_LAYER_READY); //RNL 收到这个信号后读文件

    int i, end_of_file = 0, rdsize = 0, last_buf, last_PKT_size,first_PKT=1;
    while (1)
    {

        if (ena) //如果ena
        {
            last_buf = seq_buf;
            seq_buf=1-seq_buf;
            //从链路层取数据
            sprintf(filein, "%s%04d", share_file, seq_PKT);
            inc_seq_PKT(seq_PKT);
            fin = fopen(filein, "rb");
            if (fin==NULL)
                error_exit("open file");
            rdsize = fread(buffer[seq_buf], 1, MAX_PKT, fin);
            fclose(fin);
            
            //判断是不是结束包
            end_of_file = 1;
            for (i = 0; i < MAX_PKT; i++)
                if (buffer[seq_buf][i] != '\0') //如果不是全0包，就没完。
                {
                    end_of_file = 0;
                    break;
                }
            
            //如果是结束包，处理后退出循环
            if (end_of_file) //收到结束包,buffer[last_buffer]是最后一个带数据的包,写入后退出while循环
            {
                
                last_PKT_size = 0;
                for (i = MAX_PKT - 1; i >= 0; i--) //求上一个包的最后一个非零元素位置,如果没有找到，上一个包的大小就是0（虽然不可能发生这种情况）
                    if (buffer[last_buf][i] != '\0')
                    {
                        last_PKT_size = i + 1;
                        break;
                    }
                write(fdout, buffer[last_buf], last_PKT_size);
                break;
            }
            //不是结束包，可以把上一个包完全写入接收文件中
            if (!first_PKT)//如果不是第一个包，把上一个包写入文件(必定是MAX_PKT大小)
                write(fdout, buffer[last_buf],MAX_PKT);
            else    
                first_PKT=0;

        }

        pause();
    }
    close(fdout);
}