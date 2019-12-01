#include"p2d_layer.h"
#include"tools.h"


void enable_datalink_layer(const char* proc_name)
{
    static pid=-1;
    while (pid==-1)//如果求过就不用再求了
        pid=get_first_pid(proc_name);
    sendSIG(pid,MYSIG_ENABLE_DATALINK_LAYER);

};//解除链路层阻???,使可以产生新的datalink_layer_ready事件

void disable_datalink_layer(const char* proc_name)
{
    static pid=-1;
    while (pid==-1)//如果求过就不用再求了
        pid=get_first_pid(proc_name);
    sendSIG(pid,MYSIG_DISABLE_DATALINK_LAYER);
};//使链路层阻塞,不再产生新的datalink_layer_ready事件

void enable_datalink_layer_read(const char* proc_name)
{
    static pid=-1;
    while (pid==-1)//如果求过就不用再求了
        pid=get_first_pid(proc_name);

    sendSIG(pid,MYSIG_FRAME_ARRIVAL);
//    sendSIG(pid,MYSIG_DATALINK_LAYER_READY);
}

void enable_physical_layer(const char* proc_name)
{
    static pid=-1;
    while (pid==-1)//如果求过就不用再求了
        pid=get_first_pid(proc_name);
//printf("pid:%d",pid);
    sendSIG(pid,MYSIG_ENABLE_PHYSICAL_LAYER);

};//解除物理层阻???,使可以产生新的physical_layer_ready事件

void disable_physical_layer(const char* proc_name)
{
    static pid=-1;
    while (pid==-1)//如果求过就不用再求了
        pid=get_first_pid(proc_name);
    sendSIG(pid,MYSIG_DISABLE_PHYSICAL_LAYER);
};//使物理层阻塞,不再产生新的physical_layer_ready事件

void enable_physical_layer_read(const char* proc_name)
{
    static pid=-1;
    while (pid==-1)//如果求过就不用再求了
        pid=get_first_pid(proc_name);
    sendSIG(pid,MYSIG_PHYSICAL_LAYER_READY);
}



int from_physical_layer(frame *p)
{
    static int seq_PKT=0;//使用静态局部变
    char share_file_name[MAX_FILENANE_LEN];
    int share_file=-1;
    sprintf(share_file_name, "%s%04d", PHYSICAL_DATALINK_SHARE_FILE, seq_PKT);
    inc_seq_PKT(seq_PKT);
//    printf("open phy share...\n");
    //while (share_file==-1)
        share_file = open(share_file_name, O_RDONLY);
    
    
//    printf("open phy share ok\n");
    //加锁，读取文???
    //printf("set lock to %s %d\n",share_file_name,share_file);
    //set_lock(share_file,F_RDLCK);
    
    read(share_file,p,sizeof(frame));

    //set_lock(share_file,F_UNLCK);//读完开
    close(share_file);
    printf("close share ok\n");
    
    //向DATALINK_LAYER发送enable 信号分离出去 由enable_network_layer完成
    return 0;
}



void to_physical_layer(frame *p)
{
    static int seq_PKT=0;//使用静态局部变
    char share_file_name[MAX_FILENANE_LEN];
    int share_file;
    
    sprintf(share_file_name, "%s%04d",DATALINK_PHYSICAL_SHARE_FILE, seq_PKT);
    inc_seq_PKT(seq_PKT);
    share_file = open(share_file_name, O_WRONLY | O_CREAT|O_TRUNC, 0644);

	write(share_file,p,sizeof(frame));	

    close(share_file);
}


void from_datalink_layer(frame *p)
{
	static int seq_PKT=0;//使用静态局部变
    char share_file_name[MAX_FILENANE_LEN];
    int share_file=-1;
    sprintf(share_file_name, "%s%04d", DATALINK_PHYSICAL_SHARE_FILE, seq_PKT);
    inc_seq_PKT(seq_PKT);
//    printf("open datalink share...\n");
    while (share_file==-1)
    {
        share_file = open(share_file_name, O_RDONLY);
    }
//    printf("open datalink share ok\n");
    //加锁，读取文???
    //printf("set lock to %s %d\n",share_file_name,share_file);
    set_lock(share_file,F_RDLCK);

    read(share_file,p,sizeof(frame));					

    set_lock(share_file,F_UNLCK);//读完开
    close(share_file);
//    printf("close datalink share ok\n");
    
    //向DATALINK_LAYER发送enable 信号分离出去 由enable_network_layer完成
}

void to_datalink_layer(frame *p)
{
    char share_file_name[MAX_FILENANE_LEN];
    int share_file;
    static int seq_PKT = 0;
    sprintf(share_file_name, "%s%04d", PHYSICAL_DATALINK_SHARE_FILE, seq_PKT);
        
    inc_seq_PKT(seq_PKT);
    share_file = open(share_file_name, O_WRONLY | O_CREAT|O_TRUNC, 0644);
     
    write(share_file,p,sizeof(frame));
    close(share_file);
}



// RPL is server
void connFDr(int *listenfd,int *connfd,const char* ip,const char*port)
{
	int n;
	struct sockaddr_in serv_addr;

	if((*listenfd=socket(AF_INET,SOCK_STREAM,0))==-1){
		printf("create socket error: %s(error: %d)\n",strerror(errno),errno);
		return;
	}

	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(atoi(port));

	// 端口复用
    unsigned int value = 1;
	if(setsockopt(*listenfd,SOL_SOCKET,SO_REUSEADDR,(void*)&value,sizeof(value))<0){
		perror("Fail to setsockopt\n");
		return;
	}

    if( bind(*listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1){
        printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
        return;
    }

    if( listen(*listenfd, 10) == -1){
        printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
        return;
    }


	// 此处while一直监
    while( (*connfd = accept(*listenfd, (struct sockaddr*)NULL, NULL)) == -1){
//		printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
		continue;
    }
}

int rece_phy(frame *p,int connfd)
{
	int n=0;
	while(1){
		n=recv(connfd,p,FRAMEHEAD,MSG_WAITALL);

		// ack nak packet break
		if(p->kind==nak||p->kind==ack)
			break;

		// receive packet
		recv(connfd,p->info.data,FRAMESIZE,MSG_WAITALL);
		

		break;
	}
	printf("RPL receives finished.\n");

	return n;
}

// SPL is client
void connFDs(int* sockfd,const char*ip,const char*port)
{
	int n;struct sockaddr_in  serv_addr;
    if( (*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
        return;
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(atoi(port));

    while( connect(*sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
	    continue;
    }
}
void send_phy(frame *p,int sockfd)
{
	while(1){
		// nak ack break
		if(p->kind==nak||p->kind==ack)
			send(sockfd,p,FRAMEHEAD,MSG_WAITALL);
		else
			send(sockfd,p,sizeof(frame),MSG_WAITALL);

		break;
	}

}
