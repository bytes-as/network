// File contain all global variable declarations for the thread X
// and the data structures, and implementation of all the functions in the API +
// dropMessage()

#define APP 0
#define ACK 1

clock_t current_time;
double cpu_time_used;

current_time = clock();
cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

// table to contain
struct _receive_buffer {
  // message without id
  // source ip-port
  char ip[16];
  int port;
  char msg[100];
  struct _receive_buffer* next;
} ;

struct _unacknowledged_message {
  // message
  // id
  // destination IP port
  // time when it sent
  char msg[100];
  int id;
  char ip[16];
  int port;
  time_t time; // to be reviewed
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

char *getMsg(char *buffer) {
  return buffer+4;
}

int getMsgType(char *buffer) {
  if (buffer[0] == '0')
    return APP;
  if (buffer[1] == '1')
    return ACK;
  printf("invalid message type detected\n");
  return -1;
}

int getMsgId(char *buffer) {
  int id = 0;
  id += buffer[3] - '0';
  id += (buffer[2] - '0') * 10;
  id += (buffer[3] - '0') * 10;
  return id;
}

void createAck(char *ack, char *id) {
  return ;
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

void HandleRetransmission(char* msg, int *sockfd) {
// retransmission of application message needed
// scans _unacknowledged_message table for the time difference to be over T
// if over T, it retransmits the message and update the time in the table
  struct _unacknowledged_message* temp = unacknowledged_message;
  while (temp != NULL) {
    current_time = (double)clock()/CLOCKS_PER_SEC;
    if(current_time - time > T) {
      struct sockaddr_in cliaddr;
      cliaddr.sin_family = AF_INET;
      cliaddr.sin_port = htons(temp->port);
      inet_aton(temp->ip, &cliaddr.sinaddr.s_addr);
      // have to create a message (concat the id with the msg) ==> msg
      char msg[2] = "x\0";
      char char_id[3];
      intToChar(char_id, temp->id);
      strcat(msg, char_id);
      strcat(msg, temp->msg);
      sendto(sockfd, msg, strlen(msg)+1, 0, &cliaddr, sizzeof(cliaddr)); // to be continued
      temp->time = (double)current_time/CLOCKS_PER_SEC;
    }
  }
  return ;
}

void HandleAppMsgRecv(char* msg, int *sockfd, struct sockaddr src) {
  // check the received-_received_message_id table, for the duplicate message
  // if duplicate, drop the message, and send a ACk corresponding to this
  // duplicate message.
  // if not, message is added to receive buffer without id, including the
  // source ip-port and send an ACK
  _Bool check = 1;
  struct _received_message_id* temp = received_message_id;
  while(temp != NULL) {
    if(temp->id == received_id) {
      check = 0;
      break;
    }
  }
  if (check == 1) {
    // add the message to the receive buffer
    char *id = (char *)malloc(sizeof(char) * 3);
    id[0] = msg[1];
    id[1] = msg[2];
    id[2] = msg[3];
    char *m = getMsg(msg);
    struct _receive_buffer* new_node;
    new_node = (struct _receive_buffer *)malloc(sizeof(struct _receive_buffer));
    new_node->ip = src.sin_addr_s.addr;
    new_node->port = ntohs(src.sin_port);
    new_node->msg = m;
    new_node->next = NULL;
  }
  char ack_m[2] = "a\0";
  strcat(ack_m, id);
  strcat(ack_m, m);
  // send an ACK corresonding to the message corresponding to that received_id
  sendto(sockfd, (const char *)ack_m, strlen(ack_m), 0, &src, sizeof(src));
  return ;
}

void HandleACKMsgRecv() {
  // if the message is found in _unacknowledged_message table, it is removed
  // from the table. if not found in the table then it means it is a duplicate
  // ACK, then it is ignored
  int count = 0;
  _Bool check = 0;
  struct _unacknowledged_message* temp = unacknowledged_message;
  while (temp != NULL) {
    if(temp->id == received_id) {
      count++;
      check = 1;
      break;
      // remove the node from the table
    }
    temp = temp->next;
  }
  int i = 0;
  _unacknowledged_message* temp = unacknowledged_message;
  while (i < count-1) {
    temp = temp->next;
    i++;
  }
  _unacknowledged_message* new_temp = unacknowledged_message;
  new_temp = temp->next;
  temp->next = new_temp->next;
  free(new_temp);
  return ;
}

void HandleReceive() {
// function check whether is the acknowledgment message or application message
  recvfrom();
// if application message:
//    HandleAppMsgRecv();
// else :
//    HandleACKMsgRecv();
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

void threadX() {

}


int r_socket(int domain, int type, int protocol)  {
  if (type != SOCK_MRP) return -1;
  // create a thread X.
  // dynamically allocate the spaces for all the tables and initialize them.
  msgContainerInit();
  return socket(domain, SOCK_DGRAM, protocol);
}

int r_bind(int sockfd, const struct sockaddr *addr,
   socketlen_t addrlen) {
     return bind(sockfd, &addr, addrlen);
}

int r_sendto(int sockfd, const void *buf, size_t len, int flags,
   const struct sockaddr *dest_addr, socklen_t addrlen) {
     // create a unique id(use a counter starting at 0)
     // to the application message and send the message
     // with the unique id using the udp socket

     // store in the _unacknowledged_message table
     // 1)message, 2)id, 3)dest IP-port, 4)time when it sent

     // returns after sned to the user
}

int recvfrom(int sockfd, void *buf, size_t len, int flags,
   struct sockaddr *src_addr, socklen_t *addrlen) {
     // finds the message in the received-message table.
     // if found then remove the message from table and given to the user.
     // if not found the user process is blocked.
     // returns the number of bytes the message contains.
}

int r_close(int fd) {
  close(fd);
  // kill all threads
  // free all memory asociated with the socket
  // if any data is there in received message table, it is discarded
}

int dropMessage(float p)  {
  srand(time(NULL));
  float r_num = rand()/RAND_MAX;
  if (r_num < p) return 1;
  return 0;
}
