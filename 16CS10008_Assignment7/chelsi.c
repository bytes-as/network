

Skip to content
Using Gmail with screen readers
Conversations
14.3 GB (95%) of 15 GB used
Manage
Terms · Privacy · Program Policies
Last account activity: in 1 minute
Details

// #include "rsocket.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/wait.h>
#include <errno.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>

#define T 10
#define p 0.3
#define SOCK_MRP 100
#define BUF_SIZE 100
#define TIMEOUT 10
#define DROP_PROBALITY 0.3

#define handle_error_en(en, msg) do{ errno = en; perror(msg); exit(-1);}while(0)

#define handle_error(msg) do{ perror(msg); exit(-1);} while(0)

using namespace std;
typedef struct {
	int start;
	int count;
	int capacity;
	uint32_t addr[100];
	uint16_t port[100];
	int l[100];
	char* msg[100];
}receive_buffer;
int lll;
typedef struct
{
	int id;
	time_t t;
	uint32_t addr;
	uint16_t port;
	int l;
	char* msg;
	int flags;
}unack_msg;

typedef struct {
	int start;
	int count;
	int capacity;
	unack_msg** msgs;
}unack_msg_table;



typedef struct
{
	int count;
	int capacity;
	int received_id[100];
}rec_msg_id_table;


receive_buffer* rec_buffer;
unack_msg_table* unacked_msg_tab;
rec_msg_id_table* rec_msg_ids;

char* recv_msg;
int id;
sem_t receive_id_tab;
sem_t receiv_buffer;
sem_t unack_table; // added
sem_t binding; //added
sem_t unacked_msg_tab_full;
sem_t unacked_msg_tab_empty;
sem_t receive_buffer_full;
sem_t receive_buffer_empty;
pthread_mutex_t unack_tab_mutex;

pthread_t X;
pthread_attr_t attr;




int dropMessage(){
    int r = random()%10;
    return (r < p*10);
}


int HandleRetransmit(int sockfd){
			cout <<"eentering retransmit\n";
	int i=0;
	struct timeval now;
	struct timeval limit;
	struct sockaddr_in dest_addr;
	socklen_t addrlen;
	addrlen = sizeof(dest_addr);
	cout << "In HAndleRetransmit\n";
	// sem_wait(&unack_table);
		pthread_mutex_lock(&unack_tab_mutex);

	int start = unacked_msg_tab->start;
	int count = unacked_msg_tab->count;
	while(1){
		if(i>=count)break;
		time_t now_time = time(NULL);
		if((unacked_msg_tab->msgs[(start + i)%BUF_SIZE]->t) + T > now_time){
			cout <<"&&&&&&&&&&&&&&&&&&&&&&&&&&&not retramitting "<<unacked_msg_tab->msgs[(start + i)%BUF_SIZE]->id << endl;
			break;
		}
		else{
			// Retransmit

			int len = unacked_msg_tab->msgs[(start + i)%BUF_SIZE]->l;
			char* message;
			message = (char *)malloc(sizeof(char)*(len+5));
			int idretransmit = unacked_msg_tab->msgs[(start + i)%BUF_SIZE]->id;
			cout << "idretransmit:"<<idretransmit<<endl;
		    uint32_t k = htonl(idretransmit);

		    char A = 'M';
		    memcpy(message, &A, 1);
		    memcpy(message+1, &k, 4);
		    memcpy(message+5, unacked_msg_tab->msgs[(start + i)%BUF_SIZE]->msg, len);
		    // printf("size of msd %d shul be len %ld+4\n",sizeof(message),len );
	    	dest_addr.sin_family  = AF_INET;
			dest_addr.sin_addr.s_addr = htonl(unacked_msg_tab->msgs[(start + i)%BUF_SIZE]->addr);
			dest_addr.sin_port = htons(unacked_msg_tab->msgs[(start + i)%BUF_SIZE]->port);
		    int n = sendto(sockfd, message, len+5, unacked_msg_tab->msgs[(start + i)%BUF_SIZE]->flags, (const sockaddr*)&dest_addr, addrlen);
		    if(n < len+5){
		    	perror("error in Retransmitting");
		    	return -1;
		    }
		    printf("message:%d", ntohl(*((int*)(message+1))));
		    unacked_msg_tab->msgs[(start + i)%BUF_SIZE]->t = now_time;

		    unacked_msg_tab->msgs[(unacked_msg_tab->start + unacked_msg_tab->count)%BUF_SIZE] = unacked_msg_tab->msgs[unacked_msg_tab->start];
		    // printf("BinRetransmit%d\n", unacked_msg_tab->start);
		    (unacked_msg_tab->start) = (unacked_msg_tab->start+1)%BUF_SIZE;
	// 	    printf("AinRetransmit%d\n", unacked_msg_tab->start);
		    printf("Aftersending  strt:%d\t count:%d\n",unacked_msg_tab->start,  unacked_msg_tab->count);
    		printf("###########################*************\n");
    	  for(int j=0;j<unacked_msg_tab->count;j++){
    		    printf("%d\t",unacked_msg_tab->msgs[(unacked_msg_tab->start+j)%BUF_SIZE]->id);
    	  }
    	  printf("###########################*****\n");
  			free(message);
    	}
    	i++;
    }
	// sem_post(&unack_table);
		pthread_mutex_unlock(&unack_tab_mutex);

	// cout << "Out ofHAndleRetransmit\n";
	// 	sem_getvalue(&unack_table,&lll);
			cout <<"exiting retransmit\n";

	return 0;
}

int sendanACK(int sockfd,  struct sockaddr_in cli_addr, socklen_t clilen, int id){
	char* message;
	message = (char *)malloc(sizeof(char)*(5));
    uint32_t k = htonl(id);
	char A = 'A';
    memcpy(message, &A, 1);
    memcpy(message+1, &k, 4);
    cout << "Received ACK for "<<id<<endl;
    int n = sendto(sockfd, message, 5, 0, (const sockaddr*)(&cli_addr), clilen);
    free(message);
    if(n<0)
	{
		perror("Unable to send acknowledment message");
		return -1;
	}
    return 0;
}


int HandleAppMsgRecv(int sockfd, struct sockaddr_in cli_addr, socklen_t clilen,int head, char* msg, int len){
	uint32_t headid;

	memcpy(&head, recv_msg+1, 4);
	headid = ntohl(head);
	sem_wait(&receive_id_tab);
	int a = (rec_msg_ids->received_id)[headid];
	if(a != -1){
		// Duplicate
		sem_post(&receive_id_tab);
	}
	else{

		(rec_msg_ids->received_id)[headid] = 1;
		(rec_msg_ids->count)++;
		if(rec_msg_ids->capacity <= rec_msg_ids->count){
			perror("Total received ids overflowed");

		}

		sem_post(&receive_id_tab);
		sem_wait(&receive_buffer_empty);
		sem_wait(&receiv_buffer);
		int start = rec_buffer->start;
		int count = rec_buffer->count;
		rec_buffer->msg[(start+count)%BUF_SIZE] = msg;
		rec_buffer->l[(start+count)%BUF_SIZE] = len;
		rec_buffer->addr[(start+count)%BUF_SIZE] =  ntohl(cli_addr.sin_addr.s_addr);
		rec_buffer->port[(start+count)%BUF_SIZE] =  ntohl(cli_addr.sin_port);
		rec_buffer->count++;
		cout << "port " << ntohl(cli_addr.sin_port);
		cout << "addr "<<  ntohl(cli_addr.sin_addr.s_addr);
		sem_post(&receiv_buffer);
		sem_post(&receive_buffer_full);
	}
	sendanACK(sockfd, cli_addr, clilen, headid);
}

int HandleACKMsgRecv(int k){
	cout << "******************Receiving ACK:"<<ntohl(k)<<endl;
	int i =0;
	// sem_getvalue(&unack_table,&lll);
	// printf("unack_tableb4wait:%d\n", lll);
	// sem_wait(&unack_table);
		pthread_mutex_lock(&unack_tab_mutex);

	cout <<"unack_table wait\n";
	int c = unacked_msg_tab->count;
	int start = unacked_msg_tab->start;
	for(i=0;i<c;i++){
		if(unacked_msg_tab->msgs[(start+i)%BUF_SIZE]->id == ntohl(k) ){
			cout << "`````````````````````````````````````````````````````````````````````````````````````````````````````````````Ackd for id:"<<ntohl(k) << endl;
			break;
		}
	}
	cout <<"hi";
	if(i != c){
		unack_msg* m = unacked_msg_tab->msgs[(start+i)%BUF_SIZE];
		cout <<"1";
		// free(m->msg);
		cout << "2";
		free(m);
		cout << "3\n";
		do{
			unacked_msg_tab->msgs[(start+i)%BUF_SIZE] = unacked_msg_tab->msgs[(start+i-1)%BUF_SIZE];
			i--;
		}while(i>=1);
		(unacked_msg_tab->count)--;
		printf("BACKstart%d\n", unacked_msg_tab->start);
		    (unacked_msg_tab->start) = (unacked_msg_tab->start+1)%BUF_SIZE;
		    printf("AACKstart%d\n", unacked_msg_tab->start);
	}
	// sem_post(&unack_table);
		pthread_mutex_unlock(&unack_tab_mutex);

	sem_post(&unacked_msg_tab_empty);

												// sem_wait(&receiv_buffer);
	// otherwise duplicate
	return 0;
}

int HandleReceive(int sockfd){
		// printf("memset sizeof(recv_msg):%d\n",sizeof(recv_msg) );
	memset(recv_msg,'\0',(2*BUF_SIZE));
	// puts(recv_msg+5);
	// printf("b4 recvrecv_msg[100]:%c\n",recv_msg[100] );
	struct sockaddr_in cli_addr;
	socklen_t clilen;
	// cout <<"in handlreceive\n";
	clilen = sizeof(cli_addr);
	int rb = recvfrom(sockfd, recv_msg, 2*BUF_SIZE, 0, (struct sockaddr*)&cli_addr, &clilen );

	if(rb < 5){
		perror("receival incomplete");
		exit(0);
	}
	if(rb>105){
		printf("Ignoring excess bits in message\n");
	}
	uint32_t k;
	memcpy(&k, recv_msg+1, 4);
	if(recv_msg[0]=='A'){
		HandleACKMsgRecv(k);
	}
	else if(recv_msg[0]=='M'){
		char* msg;
		msg = (char*)malloc((rb-5)*sizeof(char));
		HandleAppMsgRecv(sockfd, cli_addr, clilen, k,msg, rb-5);
	}
	else {
		perror("Undefined message");
		return -1;
	}
	return 0;

}

void* thread_X(void * arg){

	sem_wait(&binding);

	int sockfd = *((int*)arg);
	int retval;
	struct sockaddr_in cli_addr;
	int clilen = sizeof(cli_addr);
	int status;
	struct timeval tv;
	tv.tv_sec = T;
	tv.tv_usec = 0;

	fd_set readfs;

	while(1){
		FD_ZERO(&readfs);
		FD_SET(sockfd, &readfs);
		retval = select(sockfd+1, &readfs, NULL, NULL, &tv);

		if(retval == -1){
			handle_error("select");
		}
		else{
			if(FD_ISSET(sockfd, &readfs)){
				status = HandleReceive(sockfd);
				if(status!=0){
					 handle_error("Handling Receive");
				}
			}
			else{
				status = HandleRetransmit(sockfd);
				if(status!=0){
					 handle_error("Handling Receive");
				}
				tv.tv_sec = T;
				tv.tv_usec = 0;
			}
		}
	}
}


int r_socket(int domain, int type, int protocol){

	int s,i;
	if(type != SOCK_MRP){
		perror("choose SOCK_MRP next time");
		return -1;
	}
	id=0;

	int sockfd = socket(domain, SOCK_DGRAM, protocol);
	sem_init(&binding, 0 , 1);
	sem_wait(&binding);

	s = pthread_attr_init(&attr);
	if(s!=0){
		handle_error_en(s, "pthread_attr_init");
	}
	int* sockfd_to_pass = (int*)malloc(sizeof(int));
	*sockfd_to_pass = sockfd;
	s = pthread_create(&X, &attr, &thread_X, sockfd_to_pass);
	if (s != 0)
	    handle_error_en(s, "pthread_create");
	s = pthread_attr_destroy(&attr);
    if (s != 0)
        handle_error_en(s, "pthread_attr_destroy");

	sem_init(&unack_table, 0, 1);
	sem_init(&receive_id_tab, 0, 1);
	sem_init(&receiv_buffer, 0, 1);
	sem_init(&unacked_msg_tab_full, 0, 0);
	sem_init(&unacked_msg_tab_empty, 0, BUF_SIZE);
	sem_init(&receive_buffer_full, 0, 0);
	sem_init(&receive_buffer_empty, 0, BUF_SIZE);

    rec_buffer = (receive_buffer *)malloc(sizeof(receive_buffer));
    unacked_msg_tab = (unack_msg_table *)malloc(sizeof(unack_msg_table));
    unacked_msg_tab->msgs = (unack_msg**)malloc(BUF_SIZE*sizeof(unack_msg*));
    rec_msg_ids = (rec_msg_id_table *)malloc(sizeof(rec_msg_id_table));

    // free(rec_msg_ids);
    // rec_msg_ids = (rec_msg_id_table *)malloc(sizeof(rec_msg_id_table));
printf("in roskcet%p\n",rec_msg_ids );	recv_msg  = (char*)malloc((2*BUF_SIZE)*sizeof(char));
	memset(recv_msg,'\0',(2*BUF_SIZE));

    rec_buffer->count = 0;
  	rec_buffer->start = 75;
  	rec_buffer->capacity = BUF_SIZE;
    unacked_msg_tab->count = 0;
    unacked_msg_tab->start = 75;
  	unacked_msg_tab->capacity = BUF_SIZE;
    rec_msg_ids->count = 0;
    rec_msg_ids->capacity = BUF_SIZE*BUF_SIZE;

	for(int j = 0;j<rec_msg_ids->capacity;j++){
		(rec_msg_ids->received_id)[j] = -1;
	}
    char* k =  "out of r_socket";
    printf("%s\n", k);
    return sockfd;
}

int r_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
	int s = bind(sockfd, addr, addrlen);
	sem_post(&binding);
	return s;
}

ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen){
	// itoa(id, size, 10);
	char* message;
	if(len>BUF_SIZE)printf("Reducing message string length to 100\n");
	len = (len < BUF_SIZE)? len: BUF_SIZE;
	message = (char *)malloc(sizeof(char)*(len+5));
    uint32_t k = htonl(id);
    char A = 'M';
    memcpy(message, &A, 1);
    memcpy(message+1, &k, 4);
    memcpy(message+5, buf, len);
    ssize_t n = sendto(sockfd, message, len+5, flags, dest_addr, addrlen);
     // n = sendto(sockfd, message, len+5, flags, dest_addr, addrlen);
     // n = sendto(sockfd, message, len+5, flags, dest_addr, addrlen);
    if(n<5+len){
    	perror("error in transmitting complete message");
    }
    printf("message:%d\n", ntohl(*((int*)(message+1))));
    printf("n=%d\n",n );
	// struct timeval now;
    // gettimeofday(&now, NULL);
    time_t now = time(NULL);
    int start, count;
    unack_msg* m;
	m = (unack_msg*)malloc(sizeof(unack_msg));
	m->id = id;
	m->t = now;
	m->addr = ntohl(((struct sockaddr_in*)dest_addr)->sin_addr.s_addr);
	m->port = ntohs(((struct sockaddr_in*)dest_addr)->sin_port);
	m->l = len;
	m->flags = flags;
	m->msg = (char*)malloc(len);
	memcpy(m->msg,message+5 , len);
	// if(count = capacity){
	// 	//add lock to wait for acknowledge

	// }
	// sem_wait(&unacked_msg_tab_empty);
	// sem_wait(&unack_table);
	pthread_mutex_lock(&unack_tab_mutex);
	while(unacked_msg_tab->count == BUF_SIZE){
		pthread_mutex_unlock(&unack_tab_mutex);
		sleep(1);
		pthread_mutex_lock(&unack_tab_mutex);
	}

	start = unacked_msg_tab->start;
	count = unacked_msg_tab->count;
	unacked_msg_tab->msgs[(start+count)%BUF_SIZE] = m;
	unacked_msg_tab->count++;
	printf("Aftersending  strt:%d\t count:%d\n",start,  unacked_msg_tab->count);
		printf("****************************************in sendto\n");
	for(int j=0;j<unacked_msg_tab->count;j++){
		printf("%d\t",unacked_msg_tab->msgs[(start+j)%BUF_SIZE]->id);
	}
	printf("\n*************************************** in sendto\n");
	// sem_post(&unack_table);
		pthread_mutex_unlock(&unack_tab_mutex);

	// sem_post(&unacked_msg_tab_full);
	free(message);
    id++;
 //    sem_getvalue(&unack_table,&lll);
	// printf("unack_tableafter_sendto:%d\n", lll);
    return n;
}

ssize_t r_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t * addrlen){
	sem_wait(&receiv_buffer);
	while(rec_buffer->count == 0){
		sem_post(&receiv_buffer);
		sleep(1);
		sem_wait(&receiv_buffer);
	}
	// ask someone
	int start = rec_buffer->start;
	int msglen = rec_buffer->l[start];
	int minlen = (len <= msglen)? len : msglen;
	rec_buffer->count--;
	rec_buffer->start = (rec_buffer->start+1)%BUF_SIZE;

	memcpy(buf, rec_buffer->msg[(start)%BUF_SIZE], minlen);
	struct sockaddr_in* s = (struct sockaddr_in *)src_addr;
	s->sin_family = AF_INET;
	s->sin_addr.s_addr = htonl(rec_buffer->addr[(start)%BUF_SIZE]);
	s->sin_port = htons(rec_buffer->port[(start)%BUF_SIZE]);
	*addrlen = sizeof(s);
	sem_post(&receiv_buffer);
	return minlen;
}


int r_close(int fd){

	// sem_wait(&unack_table);
		pthread_mutex_lock(&unack_tab_mutex);


	while(unacked_msg_tab->count != 0)
	{
		// sem_wait(&unack_table);
			pthread_mutex_unlock(&unack_tab_mutex);

		sleep(10);
		// sem_wait(&unack_table);
			pthread_mutex_lock(&unack_tab_mutex);

	}
	cout << "unack empty"<<endl;
	sem_wait(&receive_id_tab);
	sem_wait(&receiv_buffer);
	// free(unacked_msg_tab->msgs);
	// free(unacked_msg_tab);
	free(rec_buffer);
	printf("%p\n", rec_msg_ids);
	// free(rec_msg_ids);
	// sem_post(&unack_table);
		pthread_mutex_unlock(&unack_tab_mutex);

	sem_post(&receive_id_tab);
	sem_post(&receiv_buffer);

	close(fd);
}


int main(){

	int sockfd;
	srand(time(NULL));
	struct sockaddr_in serv_addr, clie_addr;
	printf("defien\n");
	sockfd = r_socket(AF_INET, SOCK_MRP, 0);
	printf("sockfd:%d\n", sockfd);
	serv_addr.sin_family  = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(8081);

	int s =r_bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	char h[5] = "hell";
	clie_addr.sin_family  = AF_INET;
	clie_addr.sin_addr.s_addr = INADDR_ANY;
	clie_addr.sin_port = htons(8082);
	char input_string[101];
	memset(input_string,'\0',sizeof(input_string));
	scanf("%s",input_string);
	printf("%s, l =%d\n", input_string, strlen(input_string));
	for(int i =0;i<10;i++){
		printf("***********************  %d\n",i );
	r_sendto(sockfd, (const char *)input_string, strlen(input_string)+1, 0,
           (const struct sockaddr *)&clie_addr, sizeof(clie_addr));
}
	r_close(sockfd);
}

rsocket2.cpp
Displaying rsocket2.cpp.
