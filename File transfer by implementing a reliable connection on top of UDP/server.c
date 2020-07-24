#include "packet.h"

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

void die(char *s){
    perror(s);
    exit(1);
}
void printlog(int i,struct packet p){
    printf("\nServer\t");
    if(i==0){
        printf("S\t%s\t",get_current_time());
        if(p.is_data)printf("DATA\t");
        else printf("ACK \t");
        printf("%d\tSERVER\t",p.seq_no);
        if(p.seq_no%2==0)
            printf("RELAY2\t");
        else
            printf("RELAY1\t");
    }
    else if(i==1){
        printf("R\t%s\t",get_current_time());
        if(p.is_data)printf("DATA\t");
        else printf("ACK \t");
        printf("%d\t",p.seq_no);        
        if(p.seq_no%2==0)
            printf("RELAY2\t");
        else
            printf("RELAY1\t");    

        printf("SERVER\t");

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

 
int main(void){


    int opt = true; 
    struct sockaddr_in si_me, si_other;
    int s, i, slen = sizeof(si_other) , recv_len;
    //char buf[BUFLEN];
     
    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORTSERVER);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    if( setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  
          sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   
     
    //bind socket to port
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1){
        die("bind");
    }
    struct packet p;
     
    FILE * fp=fopen("output.txt","w"); 
    //keep listening for data
    while(1){
        //printf("Waiting for data...");
        fflush(stdout);
         
        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(s, &p,sizeof(p), 0, (struct sockaddr *) &si_other, &slen)) == 0){
            break;
//            die("recvfrom()");
        }
        printlog(1,p);
        p.pkt_size;
        fseek(fp,p.offset,SEEK_SET);
        fwrite(p.sendBuff,1,p.pkt_size,fp);
        
        //print details of the client/peer and the data received
        //printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        //printf("Data: %s\n" , buf);
        

        //now reply the client
        struct packet ack;
        ack.seq_no=p.seq_no;
        ack.is_data=false;
        ack.pkt_size=1;

        if (sendto(s, &ack, sizeof(struct packet), 0, (struct sockaddr*) &si_other, slen) == -1){  
            die("sendto()");
        }
        printlog(0,ack);
    }
    fclose(fp);
    close(s);
    return 0;
}

