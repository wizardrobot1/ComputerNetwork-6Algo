#include "../common/common.h"
#include "../common/tools.h"

void protocol4(void) //˫������ͬһ��Э��
{
    signal(MYSIG_NETWORK_LAYER_READY, SIG_IGN); //����MYSIG_NETWORK_LAYER_READY�ź�
    int pids[100];
    const char *network_proc = "protocol4_network";
    signal(MYSIG_NETWORK_LAYER_READY, SIG_IGN);
    while (getpid_by_name(network_proc, pids) != 3)
    {
        sleep(1); //�ȴ�������
    }
    printf("datalink ready %d\n", pids[0]);
    
    seq_nr next_frame_to_send;
    seq_nr frame_expected;
    frame r, s;
    packet buffer;
    event_type event;

    next_frame_to_send = 0; //��ʼ֡��Ϊ0
    frame_expected = 0;
    from_network_layer(&buffer,pids[0]);
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
                to_network_layer(&r.info),pids[0];
                inc(frame_expected);
            }
            if (r.ack == next_frame_to_send)//ȷ��֡���ˣ����Դ������ȡ��һ�����ˣ��رռ�ʱ��
            {
                stop_timer(r.ack);
                from_network_layer(&buffer,pids[0]); //buffer����֡
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
        start_timer(s.seq);
    }
}