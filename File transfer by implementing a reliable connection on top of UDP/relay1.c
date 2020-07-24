#include "packet.h"

int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

char * get_current_time(){
    char* str= (char*)malloc(sizeof(char)*20);
    int rc;
    time_t curr;
    struct tm* timeptr;
    struct timeval tv;

    curr=time(NULL);
    timeptr=localtime(&curr);

    gettimeofday(&tv,NULL);

    rc=strftime(str,20,"%H:%M:%S",timeptr);

    char ms[8];
    sprintf(ms,".%06ld",tv.tv_usec);
    strcat(str,ms);
    return str; 
}


void printlog(int i,struct packet p){
    fflush(stdout);
    printf("\nRELAY1\t");
    if(i==0){
        printf("S\t%s\t",get_current_time());
        if(p.is_data)printf("DATA\t");
        else printf("ACK \t");
        printf("%d\tRELAY1\t",p.seq_no);
        if(p.is_data)
            printf("SERVER\t");
        else
            printf("CLIENT\t");
    }
    else if(i==1){
        printf("R\t%s\t",get_current_time());
        if(p.is_data)printf("DATA\t");
        else printf("ACK \t");
        printf("%d\t",p.seq_no);        
        if(p.is_data)
            printf("CLIENT\t");
        else
            printf("SERVER\t");    

        printf("RELAY1\t");

    }
    else if(i==2){ 
        printf("D\t%s\t",get_current_time());
        if(p.is_data)printf("DATA\t");
        else printf("ACK \t");
        printf("%d\tRELAY1\t",p.seq_no);
        if(p.is_data)
            printf("SERVER\t");
        else
            printf("CLIENT\t");    


    }
    else {
        if(i==3)
        printf("T\t%s\t",get_current_time());
        else printf("T\t%s\t",get_current_time());
        if(p.is_data)printf("DATA\t");
        else printf("ACK \t");
        printf("%d\tCLIENT\t",p.seq_no);
        if(p.seq_no%2==0)
            printf("RELAY2\t");
        else
            printf("RELAY1\t");   
    }
    //printf("client   %s ",get_current_time(),)
    printf("\n");
}


void die(char *s)
{
 perror(s);
 exit(1);
}


float delay_Gen(){
	return  (float)rand()/(float)(RAND_MAX/2);
}

bool pdr_func(){
    int ra=rand()%100;
    if(ra>=PDR) return true;
    return false;
}


int main(){
    int opt = true; 
	struct sockaddr_in si_me, si_server,si_other,si_client;
    int s, i, slen = sizeof(si_other) , recv_len;
    //char buf[BUFLEN];
     
    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
       // return 1;
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORTRELAY1);
    si_me.sin_addr.s_addr = inet_addr("127.0.0.1");
     
    si_client.sin_family = AF_INET;
    si_client.sin_port = htons(PORTCLIENT);
    si_client.sin_addr.s_addr = inet_addr("127.0.0.1"); 

    si_server.sin_family = AF_INET;
    si_server.sin_port = htons(PORTSERVER);
    si_server.sin_addr.s_addr = inet_addr("127.0.0.1");
    //bind socket to port
    if( setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  
          sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   
    
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1){
        die("bind");
        //return 1;
    }
    struct packet p;
    while(true){
        
        if ((recv_len = recvfrom(s, &p, sizeof(p), 0, (struct sockaddr *) &si_other, &slen)) == 0){
                break;
            }
        printlog(1,p);    
        //if(pdr_func()){    
            if(p.is_data){
            	if(pdr_func()){
                    if(fork()==0){
                		msleep(delay_Gen());
                		sendto(s,&p,sizeof(p),0,(struct sockaddr *) &si_server, slen);
                	    printlog(0,p);
                        exit(1);
                    }
                }
                else{
                    printlog(2,p);  
                } 
            }
            else{
            	sendto(s,&p,sizeof(p),0,(struct sockaddr *) &si_client, slen);
                printlog(0,p);
            }
        //}
        //else printlog(2,p);

	}

    sendto(s,NULL,0,0,(struct sockaddr *) &si_server, slen);
	return 1;
}