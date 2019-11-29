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
static void wait_for_event(event_type *event) //阻塞函数，等待事件发生
{
    signal(MYSIG_FRAME_ARRIVAL, SIGHANDLER_MYSIG_FRAME_ARRIVAL);
    signal(MYSIG_TIMEOUT, SIGHANDLER_MYSIG_TIMEOUT);
    signal(MYSIG_CHSUM_ERR, SIGHANDLER_MYSIG_CHSUM_ERR);
    pause();
    *event = catch_event;
}

/*个人注解
一位滑动窗口协议：
可以同时发数据帧和确认帧
检测数据帧的序号，如果是要接收的那个序号，就传给网络层
检测确认帧的序号，如果是刚发出的帧的序号，就从网络层取下一帧（要发送的序号和要接收的序号没有联系，可能一直在发却没收，只要收到的ack信号对）
不过只要收到信号，就会发送帧，不论是否有没更新帧，同时开始倒计时，如果倒计时到了还没有外来信号，自己alarm自己，再重发一遍
*/
int main()
{
    signal(MYSIG_NETWORK_LAYER_READY, SIG_IGN); //屏蔽MYSIG_NETWORK_LAYER_READY信号
    const char *network_proc = "network";
    if (get_first_pid(network_proc)==-1)//因为要向网络层发信号提醒它读文件，所以要先打开网络层
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

    next_frame_to_send = 0; //初始帧号为0
    frame_expected = 0;
    from_network_layer(&buffer);
    enable_network_layer(network_proc);//通知网络层发下一数据

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
                to_network_layer(&r.info);
                enable_network_layer_read(network_proc);
                inc(frame_expected);
            }
            if (r.ack == next_frame_to_send)//确认帧到了，可以从网络层取下一个包了，关闭计时器
            {
                stop_timer(r.ack);
                from_network_layer(&buffer); //buffer换新帧
                enable_network_layer(network_proc);//通知网络层发下一数据
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
        start_timer(s.seq);//每次都对传出的数据包计时，如果这个序号已经在计时了，重置倒计时（同时只会对一个包计时）
    }
}