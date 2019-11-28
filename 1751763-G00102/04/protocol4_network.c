#include "../common/common.h"
#include "../common/tools.h"

static int ena_read = 0, ena_write = 1;
static event_type catch_event;
static void SIGHANDLER_MYSIG_DATALINK_LAYER_READY(int signo)
{
    ena_read = 1;
    catch_event = datalink_layer_ready;
}

static void SIGHANDLER_MYSIG_ENABLE_NETWORK_LAYER(int signo)
{
    ena_write = 1;
    catch_event = 0;
}

static void SIGHANDLER_MYSIG_DISABLE_NETWORK_LAYER(int signo)
{
    ena_write = 0;
    catch_event = 0;
}

static void wait_for_event(event_type *event) //阻塞函数，等待事件发生
{
    signal(MYSIG_DATALINK_LAYER_READY, SIGHANDLER_MYSIG_DATALINK_LAYER_READY);
    signal(MYSIG_ENABLE_NETWORK_LAYER, SIGHANDLER_MYSIG_ENABLE_NETWORK_LAYER);
    signal(MYSIG_DISABLE_NETWORK_LAYER, SIGHANDLER_MYSIG_DISABLE_NETWORK_LAYER);
    pause();
    *event = catch_event;
}

int main()
{

    //-----------------------------------------receiver---------------------------
    const char *dest_filename = "接收到的文件";
    int dest_file_fd = open(dest_filename, O_WRONLY | O_CREAT, 0644);
    if (dest_file_fd == -1)
        error_exit("open file");
    char d2n_sharefilename[MAX_FILENANE_LEN];

    FILE *d2n_sharefile;

    char d2n_buffer[2][MAX_PKT + 1]; //datalink_to_network share file 's buffer因为要判断结束包后，才会对倒二个包进行处理
    memset(d2n_buffer[0], 0, MAX_PKT + 1);
    memset(d2n_buffer[1], 0, MAX_PKT + 1);

    seq_nr d2n_seq_PKT = 0; //接下来接收那个包0000~9999
    int d2n_seq_buf = 0;    //接下来用哪个缓存区接收0~1

    int end_of_file = 0, last_buf, last_PKT_size, first_PKT = 1;

    //--------------------------------------sender-----------------------------------
    const char *share_file = NETWORK_DATALINK_SAHRE_FILE;

    char src_filename[MAX_FILENANE_LEN] = "test", fileout[MAX_FILENANE_LEN];
    //printf("请输入要传输的文件名:\n");
    //scanf("%s", src_filename);

    FILE *src_file = fopen(src_filename, "rb");
    if (src_file == NULL)
        error_exit("open file");

    char src_file_buffer[MAX_PKT + 1];
    seq_nr n2d_seq_PKT = 0;//接下来发那个包0000~9999
    int rdsize = -1;
    int n2d_sharefile_fd, lock = 0;
    //-------------------------------------common----------------------------------------
    /*
    int pids[100];
    const char *datalink_proc = "datalink";
    printf("check datalink ...\n");
    if (getpid_by_name(datalink_proc, pids) != 3)//因为要向链路层发信号，所以要先打开链路层进程
    {
        printf("请检测链路层程序是否正在运行\n");
        return 0;
    }
    printf("datalink check ok\n");
    */
    signal(MYSIG_DATALINK_LAYER_READY, SIGHANDLER_MYSIG_DATALINK_LAYER_READY); //RNL 收到这个信号后读文件
    signal(MYSIG_DISABLE_NETWORK_LAYER, SIGHANDLER_MYSIG_DISABLE_NETWORK_LAYER);
    signal(MYSIG_ENABLE_NETWORK_LAYER, SIGHANDLER_MYSIG_ENABLE_NETWORK_LAYER);

    int i;
    event_type event = 0;
    while (1)
    {

        if (ena_read) //如果ena_read ，说明链路层数据准备好了，可以去读了
        {
            last_buf = d2n_seq_buf;
            d2n_seq_buf = 1 - d2n_seq_buf;
            //从链路层取数据
            sprintf(d2n_sharefilename, "%s%04d", DATALINK_NETWORK_SAHRE_FILE, d2n_seq_PKT);
            inc_seq_PKT(d2n_seq_PKT);
            d2n_sharefile = fopen(d2n_sharefilename, "rb");
            if (d2n_sharefile == NULL)
                error_exit("open file");
            fread(d2n_buffer[d2n_seq_buf], 1, MAX_PKT, d2n_sharefile);
            fclose(d2n_sharefile);

            //判断是不是结束包
            end_of_file = 1;
            for (i = 0; i < MAX_PKT; i++)
                if (d2n_buffer[d2n_seq_buf][i] != '\0') //如果不是全0包，就没完。
                {
                    end_of_file = 0;
                    break;
                }

            //如果是结束包，处理后关闭接收文件
            if (end_of_file) //收到结束包,d2n_buffer[last_buffer]是最后一个带数据的包,写入后关闭接收文件
            {

                last_PKT_size = 0;
                for (i = MAX_PKT - 1; i >= 0; i--) //求上一个包的最后一个非零元素位置,如果没有找到，上一个包的大小就是0（虽然不可能发生这种情况）
                    if (d2n_buffer[last_buf][i] != '\0')
                    {
                        last_PKT_size = i + 1;
                        break;
                    }
                write(dest_file_fd, d2n_buffer[last_buf], last_PKT_size);
                close(dest_file_fd);
            }
            //不是结束包，可以把上一个包完全写入接收文件中
            if (!first_PKT) //如果不是第一个包，把上一个包写入文件(必定是MAX_PKT大小)
                write(dest_file_fd, d2n_buffer[last_buf], MAX_PKT);
            else
                first_PKT = 0;
            ena_read = 0; //直到下一次datalink_layer_ready 信号来之前，都不会再读
        }

        if (ena_write && rdsize && event != datalink_layer_ready) //如果ena_write而且还有的写,写入共享文件，发送ready信号
        {
            printf("ena_write\n");
            rdsize = fread(src_file_buffer, 1, MAX_PKT, src_file);
            if (rdsize % MAX_PKT || !rdsize) //倒数第二个包（文件数据的最后一个包）不是1024的整数倍，补尾0;最后一次个包全0
                for (i = rdsize; i < MAX_PKT; i++)
                    src_file_buffer[i] = '\0';

            sprintf(fileout, "%s%04d", share_file, n2d_seq_PKT);
            n2d_sharefile_fd = open(fileout, O_WRONLY | O_CREAT, 0644);
            if (n2d_sharefile_fd == -1)
                error_exit("open file");

            set_lock(n2d_sharefile_fd, F_WRLCK);
            write(n2d_sharefile_fd, src_file_buffer, MAX_PKT);
            set_lock(n2d_sharefile_fd, F_UNLCK);

            inc_seq_PKT(n2d_seq_PKT);

            printf("write share ok\n");
            //kill(pids[0], MYSIG_NETWORK_LAYER_READY);
        }

        wait_for_event(&event);

        if (event != datalink_layer_ready)//MYSIG_DISABLE_NETWORK_LAYER or MYSIG_ENABLE_NETWORK_LAYER
        {
            if (ena_write) //MYSIG_ENABLE_NETWORK_LAYER datalink成功读完share文件了,开锁，关闭文件，开始写下一个文件
            {
                if (lock)
                {
                    set_lock(n2d_sharefile_fd, F_UNLCK);
                    lock = 0;
                }
                close(n2d_sharefile_fd);
            }
            else //MYSIG_DISABLE_NETWORK_LAYER datalink 的 窗口满了，没空读share文件，加锁，并且不能写下一个文件
            {
                if (!lock) //防止重复锁同一个文件导致阻塞
                {
                    set_lock(n2d_sharefile_fd, F_WRLCK);
                    lock = 1;
                }
            }
        }

    }//end of while 
}