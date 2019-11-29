#include "../common/common.h"
#include "../common/tools.h"
#include "../common/d2n_layer.h"

#define MAX_SEQ 1
#define inc(k) if(k<MAX_SEQ) k=k+1; else k=0;
//#define MYDEBUG

static event_type catch_event;
static void SIGHANDLER_MYSIG_FRAME_ARRIVAL(int signo)
{
    catch_event = frame_arrival;
}
static void SIGHANDLER_MYSIG_TIMEOUT(int signo)
{
    catch_event = timeout;
}
static void SIGHANDLER_MYSIG_CHSUM_ERR(int signo)
{
    catch_event = cksum_err;
}
static void wait_for_event(event_type *event) //�����������ȴ��¼�����
{
    signal(MYSIG_FRAME_ARRIVAL, SIGHANDLER_MYSIG_FRAME_ARRIVAL);
    signal(MYSIG_TIMEOUT, SIGHANDLER_MYSIG_TIMEOUT);
    signal(MYSIG_CHSUM_ERR, SIGHANDLER_MYSIG_CHSUM_ERR);
    pause();
    *event = catch_event;
}

/*����ע��
һλ��������Э�飺
����ͬʱ������֡��ȷ��֡
�������֡����ţ������Ҫ���յ��Ǹ���ţ��ʹ��������
���ȷ��֡����ţ�����Ǹշ�����֡����ţ��ʹ������ȡ��һ֡��Ҫ���͵���ź�Ҫ���յ����û����ϵ������һֱ�ڷ�ȴû�գ�ֻҪ�յ���ack�źŶԣ�
����ֻҪ�յ��źţ��ͻᷢ��֡�������Ƿ���û����֡��ͬʱ��ʼ����ʱ���������ʱ���˻�û�������źţ��Լ�alarm�Լ������ط�һ��
*/
int main()
{
    signal(MYSIG_NETWORK_LAYER_READY, SIG_IGN); //����MYSIG_NETWORK_LAYER_READY�ź�
    const char *network_proc = "network";
    if (get_first_pid(network_proc)==-1)//��ΪҪ������㷢�ź����������ļ�������Ҫ�ȴ������
    {
        printf("plz start netwrok_layer first");
        return 0;
    }
    printf("datalink ready \n");
    
    seq_nr next_frame_to_send;
    seq_nr frame_expected;
    frame r, s;
    packet buffer;
    event_type event;

    next_frame_to_send = 0; //��ʼ֡��Ϊ0
    frame_expected = 0;
    from_network_layer(&buffer);
    enable_network_layer(network_proc);//֪ͨ����㷢��һ����

    s.info = buffer;
    s.seq = next_frame_to_send;
    s.ack = 1 - frame_expected;
    s.kind=data;//��ʵ��data+ack��
    to_physical_layer(&s);//������֡
    start_timer(s.seq);//�Է����İ���ʱ������ʱ���˻���alarm�жϣ�MYSIG_TIMEOUT����Ȼ����ط�

    while (true)
    {
        wait_for_event(&event);
        if (event == frame_arrival) //����Ŀ���������֡/ACK֡
        {
            from_physical_layer(&r);
            if (r.seq == frame_expected)//�����ȷ�����ϲ���
            { 
                to_network_layer(&r.info);
                enable_network_layer_read(network_proc);
                inc(frame_expected);
            }
            if (r.ack == next_frame_to_send)//ȷ��֡���ˣ����Դ������ȡ��һ�����ˣ��رռ�ʱ��
            {
                stop_timer(r.ack);
                from_network_layer(&buffer); //buffer����֡
                enable_network_layer(network_proc);//֪ͨ����㷢��һ����
                inc(next_frame_to_send);
            }
        }
        //�յ��Է�֡(�����ȷ/����)/�յ�cksum_err/timeout��Ϣ
        s.info = buffer; //��������֡���յ�ack������ȷ����Ҳ�����Ǿ�֡��û�յ����յ�ȴ�Ǵ��ack��
        //��������������֡�Ļ�������֪���Լ�����������֡��û�г�������/�ɹ����գ����ֻ��˳�����ͣ���û�����壬��Ҫ��Ϊ�˷�ack
        //����������cksum_err/timeout�����ٷ�һ��ԭ֡�����Ѷ����ٷ�һ��
        //�ظ�����ͬ�����ݰ���û�����⣬�����Է�ֻ��ȡexpect�İ�
        s.seq = next_frame_to_send;
        s.ack = 1 - frame_expected;//ֻ���յ�������֡�󷢵�ack�Ż����
        s.kind=data;
        to_physical_layer(&s);
        start_timer(s.seq);//ÿ�ζ��Դ��������ݰ���ʱ������������Ѿ��ڼ�ʱ�ˣ����õ���ʱ��ͬʱֻ���һ������ʱ��
    }
}