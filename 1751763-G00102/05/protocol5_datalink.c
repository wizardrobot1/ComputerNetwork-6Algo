#include "../common/common.h"
#include "../common/tools.h"
#include "../common/d2n_layer.h"
#include "../common/savelog.h"
#define MAX_SEQ 7 //保持2^n -1
#define inc(k) if(k<MAX_SEQ+1) k=k+1; else k=0; //k=0~7 窗口的大小是MAX_SEQ+1 , 帧序号是0~MAX_SEQ

static boolean between(seq_nr a, seq_nr b, seq_nr c)
{ //保证a<=b<c，a为循环滑动窗口下界，c为上界(三种情况)
//1.a=0 , b=5 , c=6 (待确认：0，1，2，3，4，5)
//2.c=1 , a=5 , b=6 (待确认：5，6，7,0)
//3.b=0 , c=1 , a=2 (待确认: 2,3,4,5,6,7,0)
    if (((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a)))
        return (true);
    else
        return (false);
}

static void send_data(seq_nr frame_nr, seq_nr frame_expected, packet buffer[])
{
    frame s;
    s.kind=data;//其实是数据帧捎带ack帧
    s.info = buffer[frame_nr]; //0..MAX_SEQ中的一项
    s.seq = frame_nr;
    s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1);// (frame_expected-1 + MAX_SEQ+1) % (MAX_SEQ + 1) 
    //frame_expected 是等待接收的数据包序号，dec(frame_expected)就是上一个收到的数据包序号,因为没有定义dec,所以用模运算
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
static void wait_for_event(event_type *event) //阻塞函数，等待事件发生
{
    signal(MYSIG_FRAME_ARRIVAL, SIGHANDLER_MYSIG_FRAME_ARRIVAL);
    signal(MYSIG_TIMEOUT, SIGHANDLER_MYSIG_TIMEOUT);
    signal(MYSIG_CHSUM_ERR, SIGHANDLER_MYSIG_CHSUM_ERR);
    signal(MYSIG_NETWORK_LAYER_READY, SIGHANDLER_MYSIG_NETWORK_LAYER_READY);
    pause();
    *event = catch_event;
}
/*
回退N协议
和之前几个协议不同，之前都是直接去网络层取数据，这次是要等network_layer_ready 信号来才会取数据
同时发送出的数据帧不止1个，保留所有没有得到ack的帧，超时就会重传所有保留的帧
通过限制发送的帧数量=窗口数量-1，使得每次发完所有的帧，收到的ack帧都不一样（每次应该减小1），避免了ppt的误解情况
*/

//打开顺序最后3
int main()
{
    const char *network_proc = "network";

    seq_nr next_frame_to_send;
    seq_nr ack_expected;//希望得到的ack的序号中最小的（所以发出的包的序号在[ack_expected，next_frame_to_send))
    seq_nr frame_expected;
    frame r;
    packet buffer[MAX_SEQ + 1]; //结构体数组[0..MAX_SEQ]
    seq_nr nbuffered;//已经发送出去多少个包了
    seq_nr i;
    event_type event;
    //enable_network_layer(); //允许上层发network_layer_ready，我写的网络层初始就可以发，所以不用了

    ack_expected = 0;
    next_frame_to_send = 0; //初始帧号为0
    frame_expected = 0;
    nbuffered = 0;
    while (1)
    {
        wait_for_event(&event);
        switch (event)
        { //各事件分别处理

        case network_layer_ready:
            from_network_layer(&buffer[next_frame_to_send]);
            //之前取完数据都要立即发送enable_network_layer信号通知网络层写新数据，这次是要判断窗口是否满了，如果没有满才会通知
            nbuffered = nbuffered + 1;                             //填满发送窗口
            send_data(next_frame_to_send, frame_expected, buffer); //把取到的数据发出去
            inc(next_frame_to_send);                               //发送窗上界增长
            //A发送7个包后，next_frame_to_send=7（0...6）
            //A1又发送7个包后，next_frame_to_send=6(7..5)
            //A2又发送7个包后，next_frame_to_send=6(7..5)
            break;
        case frame_arrival:
            from_physical_layer(&r);
            if (r.seq == frame_expected) //收到的数据帧的序号和等待的数据帧的序号相符，发给网络层
            {
                to_network_layer(&r.info);
                enable_network_layer_read(network_proc); //通知网络层接收
                inc(frame_expected);                     //接收序号增长,但是不发ack帧，要发数据帧的时候会顺便发，
                //A初始frame_expected=0，对面发了7帧，frame_expected=7
                //A1对面又发了7帧，frame_expected=6
                //A2对面又发了7帧，但是第0帧就出错了，frame_expected=7
            }
            else
                record_err(frame_expected,rec_mismatch_data);
            
            if (!between(ack_expected, r.ack, next_frame_to_send))
                record_err(ack_expected,rec_outrange_ack);
            while (between(ack_expected, r.ack, next_frame_to_send))           
            {//收到的r.ack是需要确认的ack中的一个，它之前的帧也可以确认（累计确认），所以要while循环从ack_expected一直确认到r.ack
                nbuffered = nbuffered - 1; //确认后，空出发送窗口
                stop_timer(ack_expected); 
                inc(ack_expected);        //发送窗下界增长,待确认的序号增加了
                //A ack_expeceted=0,next_frame_to_send=7,收到r.ack==6,-->空出7的窗口，ack_expected=7,nbuffer=0
                //A1 ack_expected=7,next_frame_to_send=6,收到r.ack==5,-->空出7的窗口，ack_expected=6
                //A2 ack_expected=7,next_frame_to_send=6,收到r.ack==6,-->不在需要确认的范围内，会超时重传
            }
            break;
        case cksum_err:
            record_err(next_frame_to_send,rec_cksum_err);
            break;
        case timeout:
            record_err(ack_expected,rec_timeout);
            record_repeat(ack_expected,nbuffered,(frame_expected + MAX_SEQ) % (MAX_SEQ + 1),rec_timeout);
            for (i = 1; i <= nbuffered; i++)//如果超过限定时间还是没有对的ack帧来，把窗口里的所有数据帧都再发一次
            {
                next_frame_to_send = ack_expected;
                send_data(next_frame_to_send, frame_expected, buffer);
                inc(next_frame_to_send);
            }
            break;
        } //end of switch

        if (nbuffered < MAX_SEQ)//如果nbuffered == MAX_SEQ ，也就是发送了7个包后，就不能再从网络层取包了
            enable_network_layer(network_proc); //允许上层发数据
        else
            disable_network_layer(network_proc); //不允许上层发数据
    }                                            // end of while
}