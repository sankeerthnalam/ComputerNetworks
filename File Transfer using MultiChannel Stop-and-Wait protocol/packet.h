#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include<time.h>
#include <stdbool.h>
#include <fcntl.h> 
#include <sys/time.h>



#define TIMEOUT 2//in sec(int)
#define PACKET_SIZE 10//in bytes
#define PDR 10//in percent



struct packet{
	size_t pkt_size;
	unsigned long offset;
	unsigned long  id;//id of the channel
	char sendBuff[PACKET_SIZE];
	bool last_packet;
	bool is_data;//if false then it is ack
	
};
