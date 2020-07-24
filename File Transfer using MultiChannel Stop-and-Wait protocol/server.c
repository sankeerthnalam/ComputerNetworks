#include "packet.h"

bool pdr_func(){
    int ra=rand()%100;
    if(ra>=PDR) return true;
    return false;
}
int max(int a,int b){
    if(a>b)return a;
    return b;
}


int main(void)
{
        int opt = true;   

    int listenfd = 0;
    int connfd1= 0,connfd2=0;
    struct sockaddr_in serv_addr;
    //char sendBuff[1025];
    int numrv;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);



//    printf("Socket retrieve success\n");

    memset(&serv_addr, '0', sizeof(serv_addr));
    //memset(sendBuff, '0', sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5001);

    if( setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  
          sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   
     


    bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));

    if(listen(listenfd, 10) == -1)
    {
        printf("Failed to listen\n");
        return -1;
    }


    FILE *fp=fopen("output.txt","w");
    int nxt_sqno=0;
    struct packet ack1,ack2,p1,p2;
    //buff.last_packet=false;
    //buff.offset=-1;
    p1.last_packet=false;
    p2.last_packet=false;

    // unsigned char offset_buffer[10] = {'\0'}; 
    // unsigned char command_buffer[2] = {'\0'}; 
    // int offset;
    // int command;
    
    connfd1 = accept(listenfd, (struct sockaddr*)NULL ,NULL);
    connfd2 = accept(listenfd, (struct sockaddr*)NULL ,NULL);
   // printf("both sockets connected\n");

    fd_set rfds;
    while(1){           
        FD_ZERO(&rfds);
        FD_SET(connfd1,&rfds);
        FD_SET(connfd2,&rfds);


        int sret=select(max(connfd1,connfd2)+1,&rfds,NULL,NULL,NULL);  

        if(FD_ISSET(connfd1,&rfds)){
            memset(&p1,0,sizeof(p1));
            read(connfd1,&p1,sizeof(p1));
            if(p1.pkt_size==0) break;
            printf("RCVD PKT: Seq.No %lu of size %zu bytes from channel %lu\n",p1.offset,p1.pkt_size,p1.id);
            if(pdr_func()){
                
                fseek(fp,p1.offset,SEEK_SET);
                fwrite(p1.sendBuff,1,p1.pkt_size,fp);    
                ack1.last_packet=p1.last_packet;
                ack1.offset=p1.offset;
                write(connfd1,&ack1,sizeof(ack1));                
                printf("SENT ACK:for PKT with Seq.No %lu from channel %lu \n",p1.offset,p1.id );             
            }
        }
        if(FD_ISSET(connfd2,&rfds)){
            memset(&p2,0,sizeof(p2));
            read(connfd2,&p2,sizeof(p2));
            if(p2.pkt_size==0) break;
            printf("RCVD PKT: Seq.No %lu of size %zu bytes from channel %lu \n",p2.offset,p2.pkt_size,p2.id );

            if(pdr_func()){
                
                ack2.last_packet=p2.last_packet;
                ack2.offset=p2.offset;
                fseek(fp,p2.offset,SEEK_SET);
                fwrite(p2.sendBuff,1,p2.pkt_size,fp);    
                write(connfd2,&ack2,sizeof(ack2));   
                printf("SENT ACK:for PKT with Seq.No %lu from channel %lu \n",p2.offset,p2.id );             
            }
            

        }

    }
    fclose(fp);


    return 0;
}


