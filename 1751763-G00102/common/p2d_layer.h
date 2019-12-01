#ifndef __P2D_LAYER_H_
#define __P2D_LAYER_H_

#define DEAFPORT	6666
#define SENIP		"192.168.80.232"
#define RECIP		"192.168.80.200"

#define TCPMSGLEN	12

#define FRAMEHEAD	12
#define FRAMESIZE	1024

#define	NETWORK		"network"
#define	DATALINK	"datalink"
#define	PHYSICAL	"physical"

#include "common.h"

void connFDr(int *listenfd,int *connfd,const char* ip,const char*port);
void connFDs(int* socket,const char* ip,const char* port);
void send_phy(frame *p,int socket);
int rece_phy(frame *p,int connfd);
void enable_datalink_layer(const char* proc_name);
void disable_datalink_layer(const char* proc_name);
void enable_datalink_layer_read(const char* proc_name);

void enable_physical_layer(const char* proc_name);
void disable_physical_layer(const char* proc_name);
void enable_physical_layer_read(const char* proc_name);

#endif
