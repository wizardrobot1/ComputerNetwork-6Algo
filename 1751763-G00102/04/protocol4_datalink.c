#include "../common/common.h"
#include "../common/tools.h"

void protocol4(void) //双方采用同一个协议
{
    signal(MYSIG_NETWORK_LAYER_READY, SIG_IGN); //屏蔽MYSIG_NETWORK_LAYER_READY信号
    int pids[100];
    const char *network_proc = "protocol4_network";
    signal(MYSIG_NETWORK_LAYER_READY, SIG_IGN);
    while (getpid_by_name(network_proc, pids) != 3)
    {
        sleep(1); //等待网络层打开
    }
    printf("datalink ready %d\n", pids[0]);
    
    seq_nr next_frame_to_send;
    seq_nr frame_expected;
    frame r, s;
    packet buffer;
    event_type event;

    next_frame_to_send = 0; //初始帧号为0
    frame_expected = 0;
    from_network_layer(&buffer,pids[0]);
    s.info = buffer;
    s.seq = next_frame_to_send;
    s.ack = 1 - frame_expected;
    s.kind=data;//其实是data+ack，
    to_physical_layer(&s);//发送首帧
    start_timer(s.seq);//对发出的包计时，倒计时到了会有alarm中断（MYSIG_TIMEOUT），然后会重发

    while (true)
    {
        wait_for_event(&event);
        if (event == frame_arrival) //到达的可能是数据帧/ACK帧
        {
            from_physical_layer(&r);
            if (r.seq == frame_expected)//序号正确则向上层送
            { 
                to_network_layer(&r.info),pids[0];
                inc(frame_expected);
            }
            if (r.ack == next_frame_to_send)//确认帧到了，可以从网络层取下一个包了，关闭计时器
            {
                stop_timer(r.ack);
                from_network_layer(&buffer,pids[0]); //buffer换新帧
                inc(next_frame_to_send);
            }
        }
        //收到对方帧(序号正确/错误)/收到cksum_err/timeout消息
        s.info = buffer; //可能是新帧（收到ack，且正确），也可能是旧帧（没收到或收到却是错的ack）
        //如果到达的是数据帧的话，并不知道自己发出的数据帧有没有出现问题/成功接收，这个只是顺带发送，并没有意义，主要是为了发ack
        //如果到达的是cksum_err/timeout，就再发一次原帧，提醒对面再发一次
        //重复传相同的数据包并没有问题，反正对方只会取expect的包
        s.seq = next_frame_to_send;
        s.ack = 1 - frame_expected;//只有收到新数据帧后发的ack才会更新
        s.kind=data;
        to_physical_layer(&s);
        start_timer(s.seq);
    }
}