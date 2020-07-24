 #include "packet.h"


 struct timeval*minim(struct timeval *t1,struct timeval *t2){

 	if(timercmp(t1,t2,<))return t1;
 	return t2;
 }
int main(void)
{
	
	int sockfd1 = 0,sockfd2=0;
	int bytesUploaded=0;//int bytesReceived = 0;

	/* Create 2 sockets */
	if((sockfd1 = socket(AF_INET, SOCK_STREAM, 0))< 0|| (sockfd2 = socket(AF_INET, SOCK_STREAM, 0))< 0)
	{
		printf("\n Error : Could not create socket \n");
		return 1;
	}

	/* Initialize sockaddr_in data structure */
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(5001); // port
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	/* Attempt a connection */
	if(connect(sockfd1, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
	{
		printf("\n Error : Connect Failed 1\n");
		return 1;
	}
	if(connect(sockfd2, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
	{
		printf("\n Error : Connect Failed 2\n");
		return 1;
	}


	FILE *fp=fopen("input.txt","r");		
	if(NULL == fp){
		  printf("Error opening file");
			return 1;
	}
	 
///////////////////////////////////////////////////////////
	fseek(fp, 0, SEEK_END); 
	int file_size=ftell(fp);
	fseek(fp, 0, SEEK_SET); 
	int no_iter;
	if(file_size%PACKET_SIZE==0) no_iter=file_size/PACKET_SIZE;
	else no_iter=file_size/PACKET_SIZE+1;

	unsigned long cur_offset=0;
	struct packet p1,p2;
	bool isover=false;
	
	size_t nread1=fread(p1.sendBuff,1,PACKET_SIZE,fp);//set this right
	size_t nread2=0;
	if(nread1<PACKET_SIZE) isover=true;
	
	if(nread1==0){
		printf("the file is empty");
		return 0;
	}

	p1.offset=(cur_offset);
	p1.id=(1);
	p1.is_data=(true);
	p1.pkt_size=(nread1);
	p1.last_packet=(isover);

	//p1temp=p1;
	if(nread1){
		write(sockfd1,&p1,sizeof(struct packet));
		printf("SENT PKT: Seq.No %lu of size %zu bytes from channel %lu\n",p1.offset,p1.pkt_size,p1.id );
		cur_offset+=PACKET_SIZE;
	}

	if(!isover){
		
		fseek(fp,cur_offset,SEEK_SET); 
		nread2=fread(p2.sendBuff,1,PACKET_SIZE,fp);
//		if(nread2!=0){
		p2.offset=cur_offset;
		p2.id=2;
		p2.is_data=true;
		p2.pkt_size=nread2;
		p2.last_packet=isover;
//		}	
		//p2temp=p2;
		//cur_offset+=PACKET_SIZE;
		if(nread2){
			write(sockfd2,&p2,sizeof(struct packet));
			printf("SENT PKT: Seq.No %lu of size %zu bytes from channel %lu\n",p2.offset,p2.pkt_size,p2.id );
			cur_offset+=PACKET_SIZE;
		}		

	}	

	struct packet ack1,ack2;

	///////////////////////
	struct timeval t1,t2,t3;
	t1.tv_sec=TIMEOUT;
	t1.tv_usec=0;
	t2=t1,t3=t1;
	int sret;
	fd_set rfds;
	bool flag=false;
	//no_iter-=2;
	//no_iter=max(0,no_iter);
	while(no_iter>0){
		FD_ZERO(&rfds);
		FD_SET(sockfd2, &rfds);
		FD_SET(sockfd1, &rfds);

		//t3=minim(t1,t2);	
		// struct timeval a;
		// gettimeofday(&a,NULL);
		// //printf("Timeout1\n");
		int sret=select( ((sockfd1>sockfd2)?sockfd1:sockfd2)+1,&rfds,NULL,NULL,&t3);
		
		//  struct timeval b;
		//  gettimeofday(&b,NULL);
		// // //t3=t3-(b-a);
		//  struct timeval c;
		//  timersub(&b,&a,&c);
		//  timersub(&t3,&c,&t3);
		  if(sret==0){
			//printf("timeout %ld :%ld\n",t3.tv_sec,t3.tv_usec);
			if(timercmp(&t1,&t2,<)){
				write(sockfd1,&p1,sizeof(struct packet));
					printf("SENT PKT: Seq.No %lu of size %zu bytes from channel %lu\n",p1.offset,p1.pkt_size,p1.id );
					
				if(flag){
					//printf("hi1\n");
					t3.tv_sec=TIMEOUT;
					t3.tv_usec=0;
				}
				else{
					timersub(&t2,&t1,&t2);
					t1.tv_sec=TIMEOUT;
					t1.tv_usec=0;	
					t3=t2;
				}
			}
			else{
				write(sockfd2,&p2,sizeof(struct packet));
				printf("SENT PKT: Seq.No %lu of size %zu bytes from channel %lu\n",p2.offset,p2.pkt_size,p2.id );
				if(flag){
					//printf("hi2\n");
					t3.tv_sec=TIMEOUT;
					t3.tv_usec=0;
				}
				else{
				timersub(&t1,&t2,&t1);
				t2.tv_sec=TIMEOUT;
				t2.tv_usec=0;
				t3=t1;
				}
			}
		//	t3=minim(t1,t2);	
		}
		else{

			if(FD_ISSET(sockfd1,&rfds)){
				no_iter--;
				read(sockfd1,&ack1,sizeof(ack1));
				printf("RCVD ACK:for PKT with Seq.No %lu from channel %d \n",ack1.offset,1 );             
				if(ack1.last_packet){
					t1.tv_sec=999999;
					t1.tv_usec=0;
					flag=true;
					continue;
				}
				if(!isover){
					memset(&p1,0,sizeof(p1));
					p1.offset=cur_offset;
					nread1=fread(p1.sendBuff,1,PACKET_SIZE,fp);
					if(nread1<PACKET_SIZE)isover=true;
					p1.id=1;
					p1.is_data=true;
					p1.pkt_size=nread1;
					p1.last_packet=isover;			
					
					if(nread1){
							write(sockfd1,&p1,sizeof(struct packet));
							printf("SENT PKT: Seq.No %lu of size %zu bytes from channel %lu\n",p1.offset,p1.pkt_size,p1.id );
							cur_offset+=p1.pkt_size;
					}
					
					// if(timercmp(&t1,&t2,<)){

					// }
					struct timeval t4;
					timersub(&t3,minim(&t1,&t2),&t4);
					timersub(&t2,&t4,&t2);//minim returns pointer
					t1.tv_sec=TIMEOUT;
					t1.tv_usec=0;	
	//				t2-=min(t1,t2)-t3
					
					t3=t2;
				}
				else{
					t1.tv_sec=999999;
					t1.tv_usec=0;
					flag=true;
					

				}

			}
			//printf("%d %d ",t3.tv_sec,t3.tv_usec);
			if(FD_ISSET(sockfd2,&rfds)&&no_iter){
				no_iter--;
				//printf("Timeout2");
				read(sockfd2,&ack2,sizeof(ack2));
				printf("RCVD ACK:for PKT with Seq.No %lu from channel %d \n",ack2.offset,2 ); 
				if(ack2.last_packet){
					t2.tv_sec=999999;
					t2.tv_usec=0;
					flag=true;
					continue;
				}


				if(!isover){
					memset(&p2,0,sizeof(p1));
					p2.offset=cur_offset;
					nread2=fread(p2.sendBuff,1,PACKET_SIZE,fp);
					if(nread2<PACKET_SIZE)isover=true;
					p2.id=2;
					p2.is_data=true;
					p2.pkt_size=nread2;
					p2.last_packet=isover;			
					if(nread2){
						write(sockfd2,&p2,sizeof(struct packet));
						cur_offset+=p2.pkt_size;
						printf("SENT PKT: Seq.No %lu of size %zu bytes from channel %lu\n",p2.offset,p2.pkt_size,p2.id );
					}
					struct timeval t4;
					timersub(&t3,minim(&t1,&t2),&t4);
					timersub(&t1,&t4,&t1);//minim returns pointer
					//timersub(&t1,minim(&t1,&t2),&t1);//minim returns pointer
					t2.tv_sec=TIMEOUT;
					t2.tv_usec=0;	
	//				t2-=min(t1,t2)-t3
					
					t3=t1;
				}
				else{
					t2.tv_sec=999999;
					t2.tv_usec=0;
					flag=true;
					

				}
	
			}
			
		}

	}
	p2.pkt_size=0;
	//memset(&p2,0,sizeof(p2));
    write(sockfd2,&p2,sizeof(struct packet));
	// while(!ack1.last_packet&&!ack2.last_packet){
	// 	FD_ZERO(&rfds);
	// 	FD_SET(sockfd2, &rfds);
	// 	FD_SET(sockfd1, &rfds);
		
	// 	t3=*minim(&t1,&t2);	

	// 	int sret=select( ((sockfd1>sockfd2)?sockfd1:sockfd2)+1,&rfds,NULL,NULL,&t3);
	
	// 	if(sret==0){
	// 		printf("timeout\n");
	// 		if(timercmp(&t1,&t2,<)){
	// 			write(sockfd1,&p1,p1.pkt_size);
	// 			printf("SENT PKT: Seq.No %d of size %d bytes from channel %d\n",p1.offset/PACKET_SIZE+1,p1.pkt_size,p1.id );
				
	// 			timersub(&t2,&t1,&t2);
	// 			t1.tv_sec=TIMEOUT;
	// 			t1.tv_usec=0;	
				
	// 			t3=t2;
	// 		}
	// 		else{
	// 			write(sockfd2,&p2,p2.pkt_size);
	// 			printf("SENT PKT: Seq.No %d of size %d bytes from channel %d\n",p2.offset/PACKET_SIZE+1,p2.pkt_size,p2.id );
	// 			timersub(&t1,&t2,&t1);
	// 			t2.tv_sec=TIMEOUT;
	// 			t2.tv_usec=0;
	// 			//t1=t3;
	// 			t3=t1;
	// 		}
	// 		//t3=minim(t1,t2);

	// 	}
	// 	else{
	// 		if(FD_ISSET(sockfd1,&rfds)){

	// 			read(sockfd1,&ack1,sizeof(ack1));
				
	// 			// if(!isover){
	// 			// 	p1.offset=cur_offset;
	// 			// 	nread1=fread(p1.sendBuff,1,PACKET_SIZE,fp);
	// 			// 	if(nread1<PACKET_SIZE)isover=true;
	// 			// 	p1.id=1;
	// 			// 	p1.is_data=true;
	// 			// 	p1.pkt_size=nread1;
	// 			// 	p1.last_packet=isover;			
	// 			// 	cur_offset+=nread1;
	// 			// 	if(nread1)write(sockfd1,&p1,p1.pkt_size);
	// 			// }


	// 		}
	// 		if(FD_ISSET(sockfd2,&rfds)){

	// 			read(sockfd2,&ack2,sizeof(ack2));
	// 			// if(!isover){
	// 			// 	p2.offset=cur_offset;
	// 			// 	nread2=fread(p2.sendBuff,1,PACKET_SIZE,fp);
	// 			// 	if(nread2<PACKET_SIZE)isover=true;
	// 			// 	p2.id=2;
	// 			// 	p2.is_data=true;
	// 			// 	p2.pkt_size=nread2;
	// 			// 	p2.last_packet=isover;			
	// 			// 	cur_offset+=nread2;
	// 			// 	if(nread2)write(sockfd2,&p2,p2.pkt_size);
	// 			// }
	
	// 		}
			

	// 	}
	// }
}