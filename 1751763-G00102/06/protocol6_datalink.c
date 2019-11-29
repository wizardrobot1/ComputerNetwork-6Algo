#include "../common/common.h"
#include "../common/tools.h"
#include "../common/d2n_layer.h"
#define MAX_SEQ 7 //����2^n -1
#define inc(k)           \
    if (k < MAX_SEQ + 1) \
        k = k + 1;       \
    else                 \
        k = 0; //k=0~7 ���ڵĴ�С��MAX_SEQ+1 , ֡�����0~MAX_SEQ
#define NR_BUFS ((MAX_SEQ + 1) / 2)//���ǿ�ͬʱ����֡��������Ҳ�ǽ��շ��Ļ�������С
boolean no_nak = true;//��¼���ڵ�ǰframe_expected�Ƿ񷢹�nak����ֹ��ͬһ��frame_expected�������nak�ش�Ҫ����frame_expected�ı������
seq_nr oldest_frame = MAX_SEQ + 1;

static boolean between(seq_nr a, seq_nr b, seq_nr c)
{ //��֤a<=b<c��aΪѭ�����������½磬cΪ�Ͻ�(�������)
    //1.a=0 , b=5 , c=6 (��ȷ�ϣ�0��1��2��3��4��5)
    //2.c=1 , a=5 , b=6 (��ȷ�ϣ�5��6��0)
    //3.b=0 , c=1 , a=2 (��ȷ��: 2,3,4,5,6,0)
    if (((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a)))
        return (true);
    else
        return (false);
}

static void send_frame(frame_kind fk, seq_nr frame_nr, seq_nr frame_expected, packet buffer[])
{
    frame s;
    s.kind = fk; //֡���ͣ�data/ack/nak����

    if (fk == data)
        s.info = buffer[frame_nr % NR_BUFS];
    s.seq = frame_nr;
    s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1);//��ȷ�ϵ����һ�����ǵȴ��İ�����һ����
    if (fk == nak)
        no_nak = false; //ȫ�ֱ���������nak��false

    to_physical_layer(&s);
    if (fk == data)
        start_timer(frame_nr % NR_BUFS); //
    stop_ack_timer();                    //���κ��������ݷ�����ֹͣack��ʱ��
}
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
static void SIGHANDLER_MYSIG_NETWORK_LAYER_READY(int signo)
{
    catch_event = network_layer_ready;
}
static void wait_for_event(event_type *event) //�����������ȴ��¼�����
{
    signal(MYSIG_FRAME_ARRIVAL, SIGHANDLER_MYSIG_FRAME_ARRIVAL);
    signal(MYSIG_TIMEOUT, SIGHANDLER_MYSIG_TIMEOUT);
    signal(MYSIG_CHSUM_ERR, SIGHANDLER_MYSIG_CHSUM_ERR);
    signal(MYSIG_NETWORK_LAYER_READY, SIGHANDLER_MYSIG_NETWORK_LAYER_READY);
    pause();
    *event = catch_event;
}
/*
�������⣺oldest_frame��ȡ�����֣�ack_timeout��timeout�¼���
*/
int main()
{
    const char *network_proc = "network";

    seq_nr ack_expected, next_frame_to_send;
    seq_nr frame_expected, too_far;//���մ��ڵ��½���Ͻ�
    int i;
    frame r;
    packet out_buf[NR_BUFS], in_buf[NR_BUFS];//���ʹ��ںͽ��մ���
    boolean arrived[NR_BUFS];//��¼0~3 �ļ����յ���
    seq_nr nbuffered;//�Ѿ������˶����˰��ˣ��ȴ�ack�İ�����
    event_type event;
    //enable_network_layer();
    ack_expected = 0;
    next_frame_to_send = 0; //��ʼ֡��Ϊ0
    frame_expected = 0;
    too_far = NR_BUFS; //too_far��Ϊ���մ��ڵ��Ͻ磬����frame_expect+NR_BUFS(���ջ������Ĵ�С)
    nbuffered = 0;
    for (i = 0; i < NR_BUFS; i++)
        arrived[i] = false;
    while (1)
    {
        wait_for_event(&event);

        switch (event)
        {
        case network_layer_ready://ͬProtocol5
            nbuffered = nbuffered + 1;
            from_network_layer(&out_buf[next_frame_to_send % NR_BUFS]);
            send_frame(data, next_frame_to_send, frame_expected, out_buf);
            inc(next_frame_to_send);
            break;
        
        case frame_arrival:
            from_physical_layer(&r);
            if (r.kind == data)
            {
                if ((r.seq != frame_expected) && no_nak)//�յ���֡������Ҫ��֡�������ɻ��ɴ�������ˣ�����nakҪ���ش�
                    send_frame(nak, 0, frame_expected, out_buf);
                else
                    start_ack_timer();//��������֡����ʱ�̿�ʼ��ʱ�����һ��ʱ���ڶ�û��˳�೵���͵�����һ��ack
                if (between(frame_expected, r.seq, too_far) && arrived[r.seq % NR_BUFS] == false)
                {
                    arrived[r.seq % NR_BUFS] = true;//��¼��֡�Ѿ�����
                    in_buf[r.seq % NR_BUFS] = r.info; //������մ���

                    while (arrived[frame_expected % NR_BUFS])//����ŵ͵�֡��ʼ����ֱ��ĳ��û�����֡��
                    {
                        to_network_layer(&in_buf[frame_expected % NR_BUFS]);
                        no_nak = true;                             //����frame_expectedǰ���ˣ������ֿ��Է�nak��
                        arrived[frame_expected % NR_BUFS] = false; //����մ���
                        inc(frame_expected);//frame_expectedǰ��
                        inc(too_far); //����frame_expected+NR_BUFS��������frame_expectedǰ��
                        start_ack_timer();
                    }
                }
            } // end of if(r.kind==data)
        
            /* ����յ�nak�������nak���е�ack���ҵ��Է�δȷ�ϵİ����Է����һ��ȷ�ϵİ����r.ack��1������ ���������Ӧ�����Լ���ȷ�ϰ�֮һ��*/
            if ((r.kind == nak) && between(ack_expected, (r.ack + 1) % (MAX_SEQ + 1), next_frame_to_send))
                send_frame(data, (r.ack + 1) % (MAX_SEQ + 1), frame_expected, out_buf);
            
            /* �����յ���ack������֡������֡�Ӵ�������*/
            while(between(ack_expected, r.ack, next_frame_to_send)) //ͬProtocol5
            {
                nbuffered = nbuffered - 1;
                stop_timer(ack_expected % NR_BUFS); //������ɾ��
                inc(ack_expected);
            }
            break;


        case cksum_err:
            if (no_nak) //û����nak��nak
                send_frame(nak, 0, frame_expected, out_buf);
            break;

        case timeout: //���ݰ���ʱ���ط����ݰ�
            send_frame(data, oldest_frame, frame_expected, out_buf);
            break;

        case ack_timeout: //ack��ʱ�򵥶���ack��(δ���Ӵ������)
            send_frame(ack, 0, frame_expected, out_buf);
            break;
    } //end of switch
    if (nbuffered < NR_BUFS)//���ͬʱ����NR_BUFS �� ��
        enable_network_layer(network_proc); //�����ϲ㷢����
    else
        disable_network_layer(network_proc); //�������ϲ㷢����
    }                                // end of while
}