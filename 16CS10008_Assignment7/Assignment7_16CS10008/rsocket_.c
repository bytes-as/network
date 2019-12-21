// File contain all global variable declarations for the thread X
// and the data structures, and implementation of all the functions in the API +
// dropMessage()

pthread_mutex_t lock;
pthread_t X;

clock_t current_time;
double cpu_time_used;


current_time = clock();
cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

// table to contain
struct _receive_buffer {
  // message without id
  // source ip-port
  uint32_t ip[100];
  uint16_t port[100];
  char msg[100];
  struct _receive_buffer* next;
} ;

struct _unacknowledged_message {
  // message, id, destination IP, Port, time when it sent
  int id;
  uint32_t ip;
  uint16_t port;
  char* msg;
  double send_time;
  struct _unacknowledged_message* next;
} ;

struct _received_message_id {
  int id;
  struct _received_message_id* next;
} ;

struct _receive_buffer* receive_buffer;
struct _unacknowledged_message* unacknowledged_message;
struct _received_message_id* received_message_id;

void msgContainerInit() {
  receive_buffer = NULL;
  unacknowledged_message = NULL;
  received_message_id = NULL;
}

char *getMsgFromBuffer(char *buffer) {
  return buffer+4;
}

int getMsgTypeFromBuffer(char *buffer) {
  if (buffer[0] == 'A')
    return 0; // for ACK MSG
  if (buffer[1] == 'M')
    return 1; // for APP MSG
  printf("invalid message type detected\n");
  return -1;
}

int getMsgIdFromBuffer(char *buffer) {
  char msg_id[3];
  msg_id[0] = buffer[1];
  msg_id[1] = buffer[2];
  msg_id[2] = buffer[3];
  int int_id = atoi(msg_id);
  return int_id;
}

// change id to char type and store it in char_id
void intToChar(char *char_id, int id) {
  // char char_id[3];
  int i=0;
  while (id) {
    char_id[2-i] = (id%10) + '0';
    id = id/10;
    i++;
  }
}

void createAck(char *ack_msg, char *id) {
  memset(ack_msg, '\0', 5*sizeof(char));
  ack_msg[0] = 'A';
  strcat(ack_msg, id);
  return ;
}

void createApp(char *app, char *app_msg, int id) {
  char char_id[3];
  intToChar(char_id, id)
  app[0] = 'M';
  strcat(app, char_id);
  strcat(app, app_msg);
  return ;
}


void HandleRetransmit(int sockfd) {
  // retransmission of application message needed
  // scans _unacknowledged_message table for the time difference to be over T
  // if over T, it retransmits the message and update the time in the table
  double current_time = (double)time()/CLOCKS_PER_SEC;
  pthread_mutex_lock(&lock);
  struct _unacknowledged_message* temp = unacknowledged_message;
  while(temp != NULL){
    if(current_time - temp->send_time >= T){
      struct sockaddr_in dest;
      dest.sin_family = AF_INET;
      dest.sin_port = htons(temp->port);
      dest.sin_addr.s_addr = htonl(temp->ip);
      printf("sending message again having id = %d, msg = %s", temp->id, temp->msg);
      char *resend_msg = (char *)malloc(105 * sizeof(char));
      createApp(resend_msg, temp->msg, temp->id);
      sendto(sockfd, resend_msg, 0, &dest, sizeof(dest));
      printf("setting up the time again for the msg having id = %d", temp->id)
      temp->send_time = current_time/CLOCKS_PER_SEC;
    }
    temp = temp->next;
  }
  pthread_mutex_unlock(&lock);
  return ;
}

void HandleAppMsgRecv(char* msg, int sockfd, struct sockaddr_in src) {
  // check the received-_received_message_id table, for the duplicate message
  // if duplicate, drop the message, and send a ACk corresponding to this
  // duplicate message.
  // if not, message is added to receive buffer without id, including the
  // source ip-port and send an ACK

  int msg_id = getMsgIdFromBuffer(msg);
  pthread_mutex_lock(&lock);
  struct _received_message_id temp = received_message_id;
  while (temp != NULL) {
    if(temp->id == msg_id) {
      struct _receive_buffer* temp = receive_buffer;
      struct _receive_buffer* new;
      new->ip = ntohl(src.sin_addr.s_addr);
      new->port = ntohl(src.sin_port);
      new->msg = getMsgFromBuffer(msg);
      new->next = temp;
      receive_buffer.next = temp;
      break;
    }
    temp = temp->next;
  }
  pthread_mutex_unlock(&lock);
  // send an ack;
  char ack_msg[5];
  createAck(ack_msg, temp->id);
  int send_byte = sendto(sockfd, ack_msg, 5, 0, (const sockaddr *)(&cliaddr), sizeof(cliaddr));
  free(ack_msg);
  if send_byte < 5) {
    perror("error sending ack message to the ");
  }
  return ;
}

void HandleACKMsgRecv(int *msg) {
  // if the message is found in _unacknowledged_message table, it is removed
  // from the table. if not found in the table then it means it is a duplicate
  // ACK, then it is ignored
  struct _unacknowledged_message* temp = unacknowledged_message;
  int msg_id;
  msg_id = getMsgIdFromBuffer(msg);
  int counter = 0;
  pthread_mutex_lock(&lock);
  while(temp != NULL) {
    count++;
    temp = temp->next;
  }
  int t = 0;
  temp = unacknowledged_message;
  for(i=0; i<counter-1; i++) {
    temp = temp->next;
  }
  struct temp_unack = temp->next;
  temp->next = temp_unack->next;
  free(temp_unack);
  pthread_mutex_unlock(&lock);
  return ;
}

void HandleReceive(int sockfd) {
// function check whether is the acknowledgment message or application message
  char *buffer;
  buffer = (char *)malloc((MAXBUFFER + 5) * sizeof(char));
  struct sockaddr_in src;
  socketlen_t src_len = sizeof(src);
  int recv_bytes = recvfrom(sockfd, buffer, MAXBUFFER + 5, 0, (struct sockaddr*)&src, &src_len);
  if(recv_bytes < 5) {
    perror("error in receiving message from the source\n");
    exit(1);
  }
// if application message:
  if(buffer[0] == 'M'){
    //    HandleAppMsgRecv();
    HandleAppMsgRecv(buffer, sockfd, src);
  }
// else :
  if(buffer[0] == 'M') {
    //    HandleACKMsgRecv();
    HandleAckMsgRecv(buffer);
  }
  else {
    printf("Error : Can't recognise the type of message...\n");
    exit(1);
  }
// on returning the thread will again wait on select call
// updating the timeout value with Trem <= T
  return ;
}

// THREAD X
// check the sockets
// put the message in the receive buffer
//
// wait on select() call to receive message from UDP socket or a timeout T
// the thread can come out of wait either on receiving a message or on a timeout
// if comes out on timeout:
//    HandleRetransmission();
// else:
//    HandleReceive();

void threadX(void* arg) {
  int sockfd = *((int *)arg);
  int r;
  fd_set fd;
  int status;

  struct timeval tv;
  tv.tv_sec = T;
  tv.tv_usec = 0;

  while (1) {
    FD_ZERO(&r);
    FD_SET(sockfd, &r);
    r = select(sockfd + 1, &readfs, NULL, NULL, &tv);

    if(r == -1) {
      printf("select error\n");
      exit(1);
    } else  {
      if(FD_ISSET(sockfd, r)) {
        status = HandleReceive(sockfd);
        if(status != 0){
          printf("handling error...\n");
          exit(1);
        } else  {
          status = HandleRetransmit(sockfd);
          if(status != 0){
            printf("handling error");
            exit(1);
          }
          tv.tv_sec = T;
          tv.tv_usec = 0;
        }
      }
    }
  }
  return ;
}


int r_socket(int domain, int type, int protocol)  {
  if (type != SOCK_MRP) return -1;
  // create a thread X.
  // dynamically allocate the spaces for all the tables and initialize them.
  msgContainerInit();
  int sockfd = socket(domain, SOCK_DGRAM, protocol);
  pthread_create(&X, NULL, &threadX, &sockfd);
  return sockfd;
}

int r_bind(int sockfd, const struct sockaddr *addr,
   socketlen_t addrlen) {
     return bind(sockfd, &addr, addrlen);
}

ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags,
   const struct sockaddr *dest_addr, socklen_t addrlen) {
     // create a unique id(use a counter starting at 0)
     // to the application message and send the message
     // with the unique id using the udp socket
     char *send_msg = (char *)malloc(105 * sizeof(char));
     send_msg[0] = 'M';
     char msg_id[3];
     intToChar(msg_id, counter);
     counter++;
     strcat(send_msg, msg_id);
     strcat(send_msg, msg);
     int send_byte = sendto(sockfd, send_msg, strlen(send_msg), flags, dest_addr, addrlen);
     if(send_byte < 5) {
       printf("Error in sending ...\n");
       exit(1);
     }
     // store in the _unacknowledged_message table
     // 1)message, 2)id, 3)dest IP-port, 4)time when it sent
     time_t current_time = time();
     struct _unacknowledged_message* new_unack;
     new_unack->id = counter-1;
     new_unack->ip = ntohl(dest_addr.sin_addr.s_addr);
     new_unack->port = ntohl(dest_addr.sin_port);
     new_unack->msg = msg;
     new_unack->send_time = current_time/CLOCKS_PER_SEC;
     pthread_mutex_lock(&lock);
     new_unack->next = unacknowledged_message;
     unacknowledged_message = new_unack;
     pthread_mutex_unlock(&lock);
     // returns after sned to the user
     return send_byte;
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
   struct sockaddr *src_addr, socklen_t *addrlen) {
     // finds the message in the received-message table.
     // if found then remove the message from table and given to the user.
     pthread_mutex_lock(&lock);
     struct _receive_buffer* temp = receive_buffer;
     if(temp == NULL) {
       while(temp == NULL)  sleep(1);
     }
     int c = 0;
     while(temp->next != NULL) {
       temp = temp->next;
       c++;
     }
     memcpy(buf, temp->msg, strlen(tmp->msg));
     temp = receive_buffer;
     int i=0;
     while(i < c-1) temp = temp->next;
     struct _received_message_id* new_temp = temp->next;
     temp->next = new_temp->next;
     free(new_temp);
     pthread_mutex_unlock(&lock);
     // if not found the user process is blocked.
     // returns the number of bytes the message contains.
     return strlen(buf);
}

int r_close(int fd) {
  close(fd);
  // kill all threads
  kill(SIG_KILL, &X);
  free(receive_buffer);
  free(unacknowledged_message);
  free(received_message_id);
  close(fd);
  // free all memory asociated with the socket
  // if any data is there in received message table, it is discarded
}

int dropMessage(float p)  {
  srand(time(NULL));
  float r_num = rand()/RAND_MAX;
  if (r_num < p) return 1;
  return 0;
}
