#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include<time.h>
#include<stdbool.h>
#include <sys/types.h> 
#include <unistd.h> 
#include<sys/time.h>
#include <time.h>
#include <errno.h>    


#define window 2  
#define TIMEOUT  2 //in seconds
#define PORTCLIENT 1233
#define PORTRELAY2 8001   //The port on which to listen for incoming data
#define PDR 10//in percent
#define PORTSERVER 8889   //The port on which to listen for incoming data
#define PORTRELAY1 8000
#define PACKET_SIZE 10


struct packet{
    int pkt_size;
    bool last_pkt;
    int offset;
    int seq_no;
    int is_data;
    char sendBuff[PACKET_SIZE];

};
