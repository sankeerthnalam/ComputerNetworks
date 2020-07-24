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


void printlog(int i,struct packet p){
    printf("\nClient\t");
    if(i==0){
        printf("S\t%s\t",get_current_time());
        if(p.is_data)printf("DATA\t");
        else printf("ACK \t");
        printf("%d\tCLIENT\t",p.seq_no);
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

        printf("CLIENT\t");

    }
    else {
        if(i==3)
        printf("T\t%s\t",get_current_time());
        else printf("RE\t%s\t",get_current_time());
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

int main(void)
{
    int opt=true;
    int wind=window;

    struct sockaddr_in si_relay1,si_relay2,si_other,si_me;
    int s, i, slen=sizeof(si_other);
    

    FILE *fp=fopen("input.txt","r");
    fseek(fp, 0, SEEK_END); 
    int file_size=ftell(fp);
    fseek(fp, 0, SEEK_SET); 
    
    int no_iter=file_size/PACKET_SIZE;


    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        die("socket");
    }
 
    memset((char *) &si_relay1, 0, sizeof(si_relay1));
    si_relay1.sin_family = AF_INET;
    si_relay1.sin_port = htons(PORTRELAY1);
    si_relay1.sin_addr.s_addr = inet_addr("127.0.0.1");



    memset((char *) &si_other, 0, sizeof(si_relay2));
    si_relay2.sin_family = AF_INET;
    si_relay2.sin_port = htons(PORTRELAY2);
    si_relay2.sin_addr.s_addr = inet_addr("127.0.0.1");

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORTCLIENT);
    si_me.sin_addr.s_addr = inet_addr("127.0.0.1"); 

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
   

    
    int nxt_sqno=0;
    int max_sqno=2*window;///this is not the max sqno it is just a mod value
    int cur_offset=0;
    int cur_sqno=0;
    struct packet p;
    struct packet buff[window];
    while(wind>0){

        int nread1=fread(p.sendBuff,1,PACKET_SIZE,fp);
        if(nread1<PACKET_SIZE)
            p.last_pkt=true;
        else
            p.last_pkt=false;
        p.seq_no=cur_sqno;
        cur_sqno=(cur_sqno+1)%max_sqno;
        p.offset=cur_offset;
        p.is_data=true;
        p.pkt_size=nread1;
        cur_offset+=p.pkt_size;
        if(nread1){
            sendto(s,&p,sizeof(struct packet),0,(struct sockaddr *) &si_relay2, slen);
            printlog(0,p);
            buff[p.seq_no%window]=p;
        }



        int nread=fread(p.sendBuff,1,PACKET_SIZE,fp);
        if(nread<PACKET_SIZE)
            p.last_pkt=true;
        else
            p.last_pkt=false;
        p.seq_no=cur_sqno;
        cur_sqno=(cur_sqno+1)%max_sqno;
        p.offset=cur_offset;
        p.is_data=true;
        p.pkt_size=nread;
        cur_offset+=p.pkt_size;
        if(nread){
            sendto(s,&p,sizeof(struct packet),0,(struct sockaddr *) &si_relay1, slen);
            printlog(0,p);
            buff[p.seq_no%window]=p;
        }

        wind=wind-2;
    }

    bool ackarr[window];
    for(int i=0;i<window;i++)ackarr[i]=false;

    wind=window;
    int max_sq=2*window;

    int b=0,e=window-1;//base and end sequence no of the window
    fd_set rfds;
    struct timeval t1;
    struct packet ack;
    
    while(no_iter){
        FD_ZERO(&rfds);
        FD_SET(s,&rfds);
        t1.tv_sec=TIMEOUT;
        t1.tv_usec=0;
      //  printf("before select\n");
        int sret= select(s+1,&rfds,NULL,NULL,&t1);
        if(sret==0){
            printf("Timeout\n");
            printlog(3,buff[b%window]);
            printlog(4,buff[b%window]);
            if(b%2==0){
                sendto(s,&buff[b%window],sizeof(struct packet),0,(struct sockaddr*) &si_relay2,slen);
            }
            else{
                sendto(s,&buff[b%window],sizeof(struct packet),0,(struct sockaddr*) &si_relay1,slen);
            }
        }
        else{
            no_iter--;
            recvfrom(s,&ack,sizeof(ack),0,(struct sockaddr*) &si_other,&slen);
            printlog(1,ack);
            if(ack.seq_no==b){
                //int count=0;
                ackarr[b%window]=true;

                while(ackarr[b%window]){
                   // printf("test\n");
                    //count++;
                    ackarr[b%window]=false;
                    b=(b+1)%max_sqno; 
                }
                
                //e=(b+window-1)%max_sqno;
                
               // e=(e+1)%max_sqno;
               while((e-b)%max_sqno!=window-1){
                    int nread3=fread(p.sendBuff,1,PACKET_SIZE,fp);
                    if(nread3<PACKET_SIZE)
                        p.last_pkt=true;
                    else
                        p.last_pkt=false;
                    p.seq_no=cur_sqno;
                    cur_sqno=(cur_sqno+1)%max_sqno;
                    p.offset=cur_offset;
                    p.is_data=true;
                    p.pkt_size=nread3;
                    cur_offset+=p.pkt_size;
                    
                    if((e+1)%2==0){
                        if(nread3){
                            sendto(s,&p,sizeof(struct packet),0,(struct sockaddr *) &si_relay2, slen);
                            printlog(0,p);
                            buff[p.seq_no%window]=p;
                        }
                        else break;
                    }
                    else{
                        if(nread3){
                            sendto(s,&p,sizeof(struct packet),0,(struct sockaddr *) &si_relay1, slen);
                            printlog(0,p);
                            buff[p.seq_no%window]=p;
                        }
                        else break;
                    }
                    e=(e+1)%max_sqno;
                }   
            }
            else{
                ackarr[ack.seq_no%window]=true;

            }

        }

    }
    sendto(s,NULL,0,0,(struct sockaddr *) &si_relay1, slen);
    sendto(s,NULL,0,0,(struct sockaddr *) &si_relay2, slen);
    close(s);
    return 0;
}

