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

static void wait_for_event(event_type *event) //�����������ȴ��¼�����
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
    const char *dest_filename = "���յ����ļ�";
    int dest_file_fd = open(dest_filename, O_WRONLY | O_CREAT, 0644);
    if (dest_file_fd == -1)
        error_exit("open file");
    char d2n_sharefilename[MAX_FILENANE_LEN];

    FILE *d2n_sharefile;

    char d2n_buffer[2][MAX_PKT + 1]; //datalink_to_network share file 's buffer��ΪҪ�жϽ������󣬲Ż�Ե����������д���
    memset(d2n_buffer[0], 0, MAX_PKT + 1);
    memset(d2n_buffer[1], 0, MAX_PKT + 1);

    seq_nr d2n_seq_PKT = 0; //�����������Ǹ���0000~9999
    int d2n_seq_buf = 0;    //���������ĸ�����������0~1

    int end_of_file = 0, last_buf, last_PKT_size, first_PKT = 1;

    //--------------------------------------sender-----------------------------------
    const char *share_file = NETWORK_DATALINK_SAHRE_FILE;

    char src_filename[MAX_FILENANE_LEN] = "test", fileout[MAX_FILENANE_LEN];
    //printf("������Ҫ������ļ���:\n");
    //scanf("%s", src_filename);

    FILE *src_file = fopen(src_filename, "rb");
    if (src_file == NULL)
        error_exit("open file");

    char src_file_buffer[MAX_PKT + 1];
    seq_nr n2d_seq_PKT = 0;//���������Ǹ���0000~9999
    int rdsize = -1;
    int n2d_sharefile_fd, lock = 0;
    //-------------------------------------common----------------------------------------
    /*
    int pids[100];
    const char *datalink_proc = "datalink";
    printf("check datalink ...\n");
    if (getpid_by_name(datalink_proc, pids) != 3)//��ΪҪ����·�㷢�źţ�����Ҫ�ȴ���·�����
    {
        printf("������·������Ƿ���������\n");
        return 0;
    }
    printf("datalink check ok\n");
    */
    signal(MYSIG_DATALINK_LAYER_READY, SIGHANDLER_MYSIG_DATALINK_LAYER_READY); //RNL �յ�����źź���ļ�
    signal(MYSIG_DISABLE_NETWORK_LAYER, SIGHANDLER_MYSIG_DISABLE_NETWORK_LAYER);
    signal(MYSIG_ENABLE_NETWORK_LAYER, SIGHANDLER_MYSIG_ENABLE_NETWORK_LAYER);

    int i;
    event_type event = 0;
    while (1)
    {

        if (ena_read) //���ena_read ��˵����·������׼�����ˣ�����ȥ����
        {
            last_buf = d2n_seq_buf;
            d2n_seq_buf = 1 - d2n_seq_buf;
            //����·��ȡ����
            sprintf(d2n_sharefilename, "%s%04d", DATALINK_NETWORK_SAHRE_FILE, d2n_seq_PKT);
            inc_seq_PKT(d2n_seq_PKT);
            d2n_sharefile = fopen(d2n_sharefilename, "rb");
            if (d2n_sharefile == NULL)
                error_exit("open file");
            fread(d2n_buffer[d2n_seq_buf], 1, MAX_PKT, d2n_sharefile);
            fclose(d2n_sharefile);

            //�ж��ǲ��ǽ�����
            end_of_file = 1;
            for (i = 0; i < MAX_PKT; i++)
                if (d2n_buffer[d2n_seq_buf][i] != '\0') //�������ȫ0������û�ꡣ
                {
                    end_of_file = 0;
                    break;
                }

            //����ǽ������������رս����ļ�
            if (end_of_file) //�յ�������,d2n_buffer[last_buffer]�����һ�������ݵİ�,д���رս����ļ�
            {

                last_PKT_size = 0;
                for (i = MAX_PKT - 1; i >= 0; i--) //����һ���������һ������Ԫ��λ��,���û���ҵ�����һ�����Ĵ�С����0����Ȼ�����ܷ������������
                    if (d2n_buffer[last_buf][i] != '\0')
                    {
                        last_PKT_size = i + 1;
                        break;
                    }
                write(dest_file_fd, d2n_buffer[last_buf], last_PKT_size);
                close(dest_file_fd);
            }
            //���ǽ����������԰���һ������ȫд������ļ���
            if (!first_PKT) //������ǵ�һ����������һ����д���ļ�(�ض���MAX_PKT��С)
                write(dest_file_fd, d2n_buffer[last_buf], MAX_PKT);
            else
                first_PKT = 0;
            ena_read = 0; //ֱ����һ��datalink_layer_ready �ź���֮ǰ���������ٶ�
        }

        if (ena_write && rdsize && event != datalink_layer_ready) //���ena_write���һ��е�д,д�빲���ļ�������ready�ź�
        {
            printf("ena_write\n");
            rdsize = fread(src_file_buffer, 1, MAX_PKT, src_file);
            if (rdsize % MAX_PKT || !rdsize) //�����ڶ��������ļ����ݵ����һ����������1024������������β0;���һ�θ���ȫ0
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
            if (ena_write) //MYSIG_ENABLE_NETWORK_LAYER datalink�ɹ�����share�ļ���,�������ر��ļ�����ʼд��һ���ļ�
            {
                if (lock)
                {
                    set_lock(n2d_sharefile_fd, F_UNLCK);
                    lock = 0;
                }
                close(n2d_sharefile_fd);
            }
            else //MYSIG_DISABLE_NETWORK_LAYER datalink �� �������ˣ�û�ն�share�ļ������������Ҳ���д��һ���ļ�
            {
                if (!lock) //��ֹ�ظ���ͬһ���ļ���������
                {
                    set_lock(n2d_sharefile_fd, F_WRLCK);
                    lock = 1;
                }
            }
        }

    }//end of while 
}