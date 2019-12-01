#include "../common/common.h"
#include "../common/tools.h"
#include "../common/d2n_layer.h"
#include "../common/savelog.h"
#define MAX_SEQ 7 //����2^n -1
#define inc(k) if(k<MAX_SEQ+1) k=k+1; else k=0; //k=0~7 ���ڵĴ�С��MAX_SEQ+1 , ֡�����0~MAX_SEQ

static boolean between(seq_nr a, seq_nr b, seq_nr c)
{ //��֤a<=b<c��aΪѭ�����������½磬cΪ�Ͻ�(�������)
//1.a=0 , b=5 , c=6 (��ȷ�ϣ�0��1��2��3��4��5)
//2.c=1 , a=5 , b=6 (��ȷ�ϣ�5��6��7,0)
//3.b=0 , c=1 , a=2 (��ȷ��: 2,3,4,5,6,7,0)
    if (((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a)))
        return (true);
    else
        return (false);
}

static void send_data(seq_nr frame_nr, seq_nr frame_expected, packet buffer[])
{
    frame s;
    s.kind=data;//��ʵ������֡�Ӵ�ack֡
    s.info = buffer[frame_nr]; //0..MAX_SEQ�е�һ��
    s.seq = frame_nr;
    s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1);// (frame_expected-1 + MAX_SEQ+1) % (MAX_SEQ + 1) 
    //frame_expected �ǵȴ����յ����ݰ���ţ�dec(frame_expected)������һ���յ������ݰ����,��Ϊû�ж���dec,������ģ����
    //Aframe_expected=7;s.ack=6
    //A1 frame_expected=6;s.ack=5
    //A2 frame_expected=7;s.ack=6
    to_physical_layer(&s);
    start_timer(frame_nr);
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
����NЭ��
��֮ǰ����Э�鲻ͬ��֮ǰ����ֱ��ȥ�����ȡ���ݣ������Ҫ��network_layer_ready �ź����Ż�ȡ����
ͬʱ���ͳ�������֡��ֹ1������������û�еõ�ack��֡����ʱ�ͻ��ش����б�����֡
ͨ�����Ʒ��͵�֡����=��������-1��ʹ��ÿ�η������е�֡���յ���ack֡����һ����ÿ��Ӧ�ü�С1����������ppt��������
*/

//��˳�����3
int main()
{
    const char *network_proc = "network";

    seq_nr next_frame_to_send;
    seq_nr ack_expected;//ϣ���õ���ack���������С�ģ����Է����İ��������[ack_expected��next_frame_to_send))
    seq_nr frame_expected;
    frame r;
    packet buffer[MAX_SEQ + 1]; //�ṹ������[0..MAX_SEQ]
    seq_nr nbuffered;//�Ѿ����ͳ�ȥ���ٸ�����
    seq_nr i;
    event_type event;
    //enable_network_layer(); //�����ϲ㷢network_layer_ready����д��������ʼ�Ϳ��Է������Բ�����

    ack_expected = 0;
    next_frame_to_send = 0; //��ʼ֡��Ϊ0
    frame_expected = 0;
    nbuffered = 0;
    while (1)
    {
        wait_for_event(&event);
        switch (event)
        { //���¼��ֱ���

        case network_layer_ready:
            from_network_layer(&buffer[next_frame_to_send]);
            //֮ǰȡ�����ݶ�Ҫ��������enable_network_layer�ź�֪ͨ�����д�����ݣ������Ҫ�жϴ����Ƿ����ˣ����û�����Ż�֪ͨ
            nbuffered = nbuffered + 1;                             //�������ʹ���
            send_data(next_frame_to_send, frame_expected, buffer); //��ȡ�������ݷ���ȥ
            inc(next_frame_to_send);                               //���ʹ��Ͻ�����
            //A����7������next_frame_to_send=7��0...6��
            //A1�ַ���7������next_frame_to_send=6(7..5)
            //A2�ַ���7������next_frame_to_send=6(7..5)
            break;
        case frame_arrival:
            from_physical_layer(&r);
            if (r.seq == frame_expected) //�յ�������֡����ź͵ȴ�������֡�������������������
            {
                to_network_layer(&r.info);
                enable_network_layer_read(network_proc); //֪ͨ��������
                inc(frame_expected);                     //�����������,���ǲ���ack֡��Ҫ������֡��ʱ���˳�㷢��
                //A��ʼframe_expected=0�����淢��7֡��frame_expected=7
                //A1�����ַ���7֡��frame_expected=6
                //A2�����ַ���7֡�����ǵ�0֡�ͳ����ˣ�frame_expected=7
            }
            else
                record_err(frame_expected,rec_mismatch_data);
            
            if (!between(ack_expected, r.ack, next_frame_to_send))
                record_err(ack_expected,rec_outrange_ack);
            while (between(ack_expected, r.ack, next_frame_to_send))           
            {//�յ���r.ack����Ҫȷ�ϵ�ack�е�һ������֮ǰ��֡Ҳ����ȷ�ϣ��ۼ�ȷ�ϣ�������Ҫwhileѭ����ack_expectedһֱȷ�ϵ�r.ack
                nbuffered = nbuffered - 1; //ȷ�Ϻ󣬿ճ����ʹ���
                stop_timer(ack_expected); 
                inc(ack_expected);        //���ʹ��½�����,��ȷ�ϵ����������
                //A ack_expeceted=0,next_frame_to_send=7,�յ�r.ack==6,-->�ճ�7�Ĵ��ڣ�ack_expected=7,nbuffer=0
                //A1 ack_expected=7,next_frame_to_send=6,�յ�r.ack==5,-->�ճ�7�Ĵ��ڣ�ack_expected=6
                //A2 ack_expected=7,next_frame_to_send=6,�յ�r.ack==6,-->������Ҫȷ�ϵķ�Χ�ڣ��ᳬʱ�ش�
            }
            break;
        case cksum_err:
            record_err(next_frame_to_send,rec_cksum_err);
            break;
        case timeout:
            record_err(ack_expected,rec_timeout);
            record_repeat(ack_expected,nbuffered,(frame_expected + MAX_SEQ) % (MAX_SEQ + 1),rec_timeout);
            for (i = 1; i <= nbuffered; i++)//��������޶�ʱ�仹��û�жԵ�ack֡�����Ѵ��������������֡���ٷ�һ��
            {
                next_frame_to_send = ack_expected;
                send_data(next_frame_to_send, frame_expected, buffer);
                inc(next_frame_to_send);
            }
            break;
        } //end of switch

        if (nbuffered < MAX_SEQ)//���nbuffered == MAX_SEQ ��Ҳ���Ƿ�����7�����󣬾Ͳ����ٴ������ȡ����
            enable_network_layer(network_proc); //�����ϲ㷢����
        else
            disable_network_layer(network_proc); //�������ϲ㷢����
    }                                            // end of while
}