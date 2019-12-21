// TODO : check for strcpy conflicts with other message formats (suppose someone is sending a 0 or the id is 0)
#include "rsocket.h"
#define T 2
#define P 0.1

int retranmission_count = 0;

// #define TESTING

#ifdef TESTING

pthread_mutex_t printLock = PTHREAD_MUTEX_INITIALIZER;

#define x_debug_log(...)  pthread_mutex_lock(&printLock);printf("\033[33m");printf(__VA_ARGS__);printf("\033[0m");pthread_mutex_unlock(&printLock);fflush(stdout);fflush(stdin)
#define debug_log(...)  pthread_mutex_lock(&printLock);printf("\033[36m");printf(__VA_ARGS__);printf("\033[0m");pthread_mutex_unlock(&printLock);fflush(stdout);fflush(stdin)
#define FLUSH()  fflush(stdin);fflush(stdout);fflush(stderr)

#else

#define x_debug_log(...)
#define debug_log(...)
#define FLUSH()

#endif


struct def_Node {
    int key;
    void * container;
    struct def_Node * next;
};
typedef struct def_Node Node;
typedef struct def_Node * Table;

Node * create_Node(long int key, void * container){
    Node * ptr = (Node *)malloc(sizeof(Node));
    ptr->container = container;
    ptr->key = key ;
    ptr->next = NULL;
    return ptr;
}

Node * insert_node(Node * root, int key, void * container){
    Node * returnval = root;
    // printf("inserting value %d\n", key);
    if(root == NULL)
        return create_Node(key, container);

    while(root != NULL){
        if(root->next == NULL){
            root->next = create_Node(key, container);
            root->next->next = NULL;
            return returnval;
        }
        root = root->next;
    }
}

Node * delete_node(Node * root, int key){
    Node * returnval = root;
    if(root == NULL)
        return NULL;
    if(root->key == key){
        Node * ptr = root;
        if(root->container != NULL){
            free(root->container);
        }
        free(root);
        return root->next;
    }
    while(root != NULL){
        if(root->next == NULL)
            break;
        else if(root->next->key == key){
            Node * ptr = root->next;
            root->next = ptr->next;
            free(ptr);
            break;
        }
        root = root->next;
    }
    return returnval;
}


Node * find_node(Node * root, int key){
    if(root == NULL)
        return NULL;

    while(root != NULL){
        if(root->key == key)
            return root;
        else
            root = root->next;
    }
    return NULL;
}

void print_list(Node * root){
    while(root!=NULL){
        printf("%ld ", root->key);
        root = root->next;
    }
}

Node * delete_list(Node * root){
    if(root == NULL){
        return NULL;
    }
    delete_list(root->next);
    free(root);
    return NULL;
}

void * apply_to_all_nodes(Node * root, void function_ptr(Node * ptr, void * args), void * args){
    while(root != NULL){
        function_ptr(root, args);
        root = root->next;
    }
}

int length(Node * root){
    int length_val = 0 ;
    while(root != NULL){
        root = root->next;
        length_val++;
    }
    return length_val;
}

struct def_receive_buffer_container{
    int message_id;
    char buffer[100];
    struct sockaddr addr;
    int len;
    int addrlen;
};

struct def_unacknowledge_message_container{
    int message_id;
    char buffer[100];
    struct sockaddr addr;
    time_t sending_time;
};

struct def_recieved_message_container{
    int message_id;
};

struct def_socket_container{
    Table receive_message;
    Table unacknowledged_message;
    Table receive_message_id;

    int sockfd;
    int message_counter;
    int is_usable;
    int is_close_requested;

    pthread_mutex_t lock;
    pthread_cond_t cond;
};

struct def_thread_container{
    pthread_t thread_id;
    // pthread_mutex_t lock;
    pthread_attr_t attr;
};

typedef struct def_receive_buffer_container receive_message_container ;
typedef struct def_unacknowledge_message_container unacknowledge_message_container ;
typedef struct def_recieved_message_container receive_message_id_container ;
typedef struct def_socket_container socket_container ;
typedef struct def_thread_container thread_container ;

int dropMessage(float p);

receive_message_id_container * new_receive_message_id_container(int message_id){
    receive_message_id_container * ptr = (receive_message_id_container *)malloc(sizeof(receive_message_id_container));
    ptr->message_id = message_id;
    return ptr;
}

receive_message_container * new_receive_message_container(int message_id, char buffer[100]){
    receive_message_container * buffer_ptr = (receive_message_container *)malloc(sizeof(receive_message_container));
    buffer_ptr->message_id = message_id;
    strcpy(buffer_ptr->buffer, buffer);

    debug_log("receive buffer container constructed \n");
    return buffer_ptr;
}

unacknowledge_message_container * new_unacknowledge_message_container(){
    unacknowledge_message_container * ptr = (unacknowledge_message_container *)malloc(sizeof(unacknowledge_message_container));
    memset(ptr, 0, sizeof(unacknowledge_message_container));

    debug_log("unacknowledge message container constructed \n");
    return ptr;
}

int send_unacknowledge_message(int sockfd, unacknowledge_message_container * ptr){
    debug_log("sending message \n ");
    char * buffer = (char *)malloc(sizeof(char) * (strlen(ptr->buffer) + 2 ));
    int i=1 ;
    for(i =1 ; i < strlen(ptr->buffer) + 1 ; i++){
        buffer[i] = ptr->buffer[i - 1];
    }
    buffer[i] = '\0';
    buffer[i+1] = '\0';

    buffer[0] = (char)ptr->message_id;
    int send_n = sendto(sockfd, buffer, strlen(ptr->buffer + 1) + 3, 0, &ptr->addr, sizeof(ptr->addr));
    return send_n;
}

void sockaddr_copy(struct sockaddr * base, struct sockaddr * from){
    base->sa_family = from->sa_family;
    int i=0 ;
    for(i =0 ; i < 14 ; i++){
        base->sa_data[i] = from->sa_data[i];
    }
}

socket_container * new_socket_container(int sockfd){
    socket_container * socket_ptr = (socket_container * )malloc(sizeof(socket_container));

    socket_ptr->message_counter = 0;
    socket_ptr->is_usable = 0;
    socket_ptr->is_close_requested =  0;

    socket_ptr->receive_message = NULL;
    socket_ptr->receive_message_id = NULL;
    socket_ptr->unacknowledged_message = NULL;

    socket_ptr->sockfd = sockfd;

    pthread_mutex_init(&socket_ptr->lock, NULL);
    pthread_cond_init(&socket_ptr->cond, NULL);

    debug_log("new socket container constructed \n");
    return socket_ptr;
}

thread_container * new_thread_container(){

    thread_container * thread_ptr = (thread_container *)malloc(sizeof(thread_container));
    // pthread_mutex_init(&thread_ptr->lock, NULL);
    pthread_attr_init(&thread_ptr->attr);
    pthread_attr_setdetachstate(&thread_ptr->attr, PTHREAD_CREATE_DETACHED);

    debug_log("new thread container constructed \n");
    return thread_ptr;
}


Table socket_table = NULL;
Table thread_table = NULL;


pthread_mutex_t socket_table_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t thread_table_lock = PTHREAD_MUTEX_INITIALIZER;


pthread_cond_t socket_table_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t thread_table_cond = PTHREAD_COND_INITIALIZER;

void * thread_x_routine(void * args);


int r_socket(int domain, int type, int protocol){
    srand((unsigned long int)time(NULL));
    int sockfd = socket(domain, type, protocol);
    if(sockfd == -1)
        return -1;

    socket_container * socket_ptr = new_socket_container(sockfd);
    thread_container * thread_ptr = new_thread_container();

    int ret = pthread_create(&thread_ptr->thread_id, &thread_ptr->attr, thread_x_routine, (void *)socket_ptr);
    if(ret == -1){
        pthread_mutex_destroy(&socket_ptr->lock);

        // pthread_mutex_destroy(&thread_ptr->lock);
        pthread_attr_destroy(&thread_ptr->attr);

        free(socket_ptr);
        free(thread_ptr);

        return -1;
    }

    pthread_mutex_lock(&socket_table_lock);
    socket_table = insert_node(socket_table, sockfd, (void *)socket_ptr);
    pthread_mutex_unlock(&socket_table_lock);

    pthread_mutex_lock(&thread_table_lock);
    thread_table = insert_node(thread_table, sockfd, (void *)thread_ptr);
    pthread_mutex_unlock(&thread_table_lock);

    return sockfd;
}

int r_bind(int fd, const struct sockaddr * addr, socklen_t len){
    pthread_mutex_lock(&socket_table_lock);
    Node * node1 =  find_node(socket_table, fd);
    pthread_mutex_unlock(&socket_table_lock);

    if(node1 == NULL)
        return -1;
    socket_container * socket_ptr = (socket_container *)node1->container;

    if(bind(fd, addr, len)  < 0){
        return -1 ;
    }
    // debug_log("\nfetching lock for socket_ptr");
    pthread_mutex_lock(&socket_ptr->lock);
    socket_ptr->is_usable = 1 ;
    debug_log("setting thread free\n");
    pthread_cond_signal(&socket_ptr->cond);
    pthread_mutex_unlock(&socket_ptr->lock);

    return 1;
}

int r_connect(int fd, const struct sockaddr * addr, socklen_t len){
    pthread_mutex_lock(&socket_table_lock);
    Node * node1 =  find_node(socket_table, fd);
    pthread_mutex_unlock(&socket_table_lock);

    if(node1 == NULL)
        return -1;
    socket_container * socket_ptr = (socket_container *)node1->container;

    if(connect(fd, addr, len)  < 0){
        return -1 ;
    }

    pthread_mutex_lock(&socket_ptr->lock);
    socket_ptr->is_usable = 1 ;
    pthread_cond_signal(&socket_ptr->cond);
    pthread_mutex_unlock(&socket_ptr->lock);

    return 1;
}

int r_sendto(int fd, const void * buf, size_t len, int flag, const struct sockaddr * addr, socklen_t addrlen){
    pthread_mutex_lock(&socket_table_lock);
    Node * node1 =  find_node(socket_table, fd);
    pthread_mutex_unlock(&socket_table_lock);

    if(node1 == NULL)
        return -1;
    socket_container * socket_ptr = (socket_container *)node1->container;


    unacknowledge_message_container * unack_message_ptr = new_unacknowledge_message_container();
    strcpy(unack_message_ptr->buffer, (char *)buf);
    unack_message_ptr->message_id = socket_ptr->message_counter;
    sockaddr_copy(&unack_message_ptr->addr, addr);
    time(&unack_message_ptr->sending_time);

    int ret = send_unacknowledge_message(fd, unack_message_ptr);
    if(ret == -1){
        free(unack_message_ptr);
        return -1;
    }

    socket_ptr->message_counter++;

    // debug_log("\nfetching lock for socket_ptr");
    pthread_mutex_lock(&socket_ptr->lock);
    socket_ptr->unacknowledged_message = insert_node(socket_ptr->unacknowledged_message, unack_message_ptr->message_id, unack_message_ptr);
    pthread_mutex_unlock(&socket_ptr->lock);

    return ret;
}

int r_close(int fd){
    pthread_mutex_lock(&socket_table_lock);
    Node * node1 =  find_node(socket_table, fd);
    pthread_mutex_unlock(&socket_table_lock);

    pthread_mutex_lock(&socket_table_lock);
    Node * node2 =  find_node(thread_table, fd);
    pthread_mutex_unlock(&socket_table_lock);

    if(node1 == NULL)
        return -1;
    socket_container * socket_ptr = (socket_container *)node1->container;
    thread_container * thread_ptr = (thread_container *)node2->container;
    // debug_log("\nfetching lock for socket_ptr");

    while(length(socket_ptr->unacknowledged_message) > 0){
        debug_log("Waiting for clearance of unacknowledge message buffer\n");
        sleep(1);
    }

    pthread_mutex_lock(&socket_ptr->lock);
    socket_ptr->is_close_requested = 1 ;
    debug_log("requesting thread to close\n");
    pthread_cond_wait(&socket_ptr->cond, &socket_ptr->lock);
    pthread_mutex_unlock(&socket_ptr->lock);

    pthread_attr_destroy(&thread_ptr->attr);

    pthread_mutex_destroy(&socket_ptr->lock);
    pthread_cond_destroy(&socket_ptr->cond);

    pthread_mutex_lock(&socket_table_lock);
    socket_table = delete_node(socket_table, fd);
    pthread_mutex_unlock(&socket_table_lock);

    pthread_mutex_lock(&thread_table_lock);
    thread_table = delete_node(thread_table, fd);
    pthread_mutex_unlock(&thread_table_lock);

    return close(fd);
}

void retransmit(Node * ptr, void * args){
    unacknowledge_message_container * unack_message = (unacknowledge_message_container *)ptr->container;
    if(time(NULL) - unack_message->sending_time > T){
        x_debug_log("Retransmitting    : %s\n", unack_message->buffer);
        x_debug_log("Retransmitting to : %d\n", *((int *)args));
        send_unacknowledge_message(*((int *)args), unack_message);
        unack_message->sending_time = time(NULL);
        retranmission_count++;
        x_debug_log("retransmitting message\n");
    }
    return;
}

void HandleRetansmission(socket_container * ptr){
    pthread_mutex_lock(&ptr->lock);
    x_debug_log("Handling retransmission\n");
    apply_to_all_nodes(ptr->unacknowledged_message, retransmit, (void *)&ptr->sockfd);
    pthread_mutex_unlock(&ptr->lock);
}



void HandleACKMsgRecv(socket_container *ptr, char * tempbuffer, int len){
    x_debug_log("Handeling Acknowledge message \n");
    int message_id = (int)tempbuffer[1];
    x_debug_log("Message received  = %d \n", message_id);

    pthread_mutex_lock(&ptr->lock);
    ptr->unacknowledged_message = delete_node(ptr->unacknowledged_message, message_id);
    pthread_mutex_unlock(&ptr->lock);

    return ;
}

void HandleAppMsgRecv(socket_container *ptr, char * tempbuffer, int len, struct sockaddr temp, int addrlen){
    x_debug_log("Handling Application message (received)\n");

    int message_id = (int)tempbuffer[0];
    char acknowledgement_buffer[3];

    Node  * node = find_node(ptr->receive_message_id, message_id);

    if(node != NULL){
        acknowledgement_buffer[0] = 'a';
        acknowledgement_buffer[1] = (char)message_id;
        acknowledgement_buffer[2] = '\0';
        int send_n = sendto(ptr->sockfd, acknowledgement_buffer, 3, 0, &temp, addrlen);

        x_debug_log("Duplicate message received \n");

        return ;
    }

    receive_message_id_container * id_container = new_receive_message_id_container(message_id);

    receive_message_container * recvmsg_container = new_receive_message_container(message_id, tempbuffer + 1);
    recvmsg_container->addrlen = addrlen;
    recvmsg_container->len = len;
    recvmsg_container->addr = temp;

    pthread_mutex_lock(&ptr->lock);

    ptr->receive_message_id = insert_node(ptr->receive_message_id, id_container->message_id , (void *)id_container);

    ptr->receive_message = insert_node(ptr->receive_message, recvmsg_container->message_id, (void *)recvmsg_container);

    x_debug_log("Inserted a received message in the buffer \n");
    x_debug_log("Buffer   : %s \n", recvmsg_container->buffer);
    x_debug_log("Buffer-id: %d \n", recvmsg_container->message_id);


    pthread_cond_signal(&ptr->cond);
    pthread_mutex_unlock(&ptr->lock);


    acknowledgement_buffer[0] = 'a';
    acknowledgement_buffer[1] = (char)message_id;
    acknowledgement_buffer[2] = '\0';
    int send_n = sendto(ptr->sockfd, acknowledgement_buffer, 3, 0, &temp, addrlen);

    x_debug_log("Message received \n");

    x_debug_log("Sending Acknowledgement\n");

    return ;
}



void HandleReceive(socket_container * ptr){

    x_debug_log("handling receive \n");
    char tempbuffer[101];
    int len = sizeof(struct sockaddr );
    struct sockaddr temp ;

    int recv_n = recvfrom(ptr->sockfd, tempbuffer, 101, 0, &temp, &len);

    if(dropMessage(P)){
        x_debug_log("Dropping Message\n");
        return ;
    }

    if(tempbuffer[0] == 'a'){
        HandleACKMsgRecv(ptr, tempbuffer, recv_n);
    }
    else{
        HandleAppMsgRecv(ptr, tempbuffer, recv_n, temp, len);
    }
}

void * thread_x_routine(void * args){
    x_debug_log("thread created and running\n");
    socket_container * socket_ptr = (socket_container * )args;

    // x_debug_log("\nfetching lock for socket_ptr");
    pthread_mutex_lock(&socket_ptr->lock);
    if(socket_ptr->is_usable == 0){
        x_debug_log("thread waiting for socket to become usable\n");
        pthread_cond_wait(&socket_ptr->cond, &socket_ptr->lock);
        x_debug_log("thread socket became usable\n");
    }
    pthread_mutex_unlock(&socket_ptr->lock);

    fd_set rset;
    FD_ZERO(&rset);

    int maxfd = socket_ptr->sockfd + 1 , select_ret;
    struct timeval timeout;
    timeout.tv_sec = T;
    timeout.tv_usec = 0;

    int quit_condition = 0 ;
    while(quit_condition == 0){
        // x_debug_log("\nlocking socket_container\n");
        pthread_mutex_lock(&socket_ptr->lock);
        // x_debug_log("\nsocket container locked\n");
        if(socket_ptr->is_close_requested == 1){
            quit_condition = 1;

            // TODO : waiting till all the messages have ben recieved

            socket_ptr->receive_message = delete_list(socket_ptr->receive_message);
            socket_ptr->receive_message_id = delete_list(socket_ptr->receive_message_id);
            socket_ptr->unacknowledged_message = delete_list(socket_ptr->unacknowledged_message);

            x_debug_log("close request handled by thread\n");
            pthread_cond_signal(&socket_ptr->cond);
        }
        pthread_mutex_unlock(&socket_ptr->lock);
        if(quit_condition == 1 )
        {
            break;
        }

        // select statements

        FD_SET(socket_ptr->sockfd, &rset);
        select_ret = select(maxfd, &rset, NULL, NULL, &timeout);
        if(select_ret == -1){
            x_debug_log("Something went wrong somewhere \n");
        }
        else if(select_ret == 0){
            x_debug_log("Time out occured\n");

            HandleRetansmission(socket_ptr);

            timeout.tv_sec = T;
            timeout.tv_usec = 0;
        }
        else if(FD_ISSET(socket_ptr->sockfd, &rset)){
            x_debug_log("something received \n");
            HandleReceive(socket_ptr);
        }
    }
    return NULL;
}


int r_recvfrom(int fd, void * buffer, size_t len, int flag, struct sockaddr * addr , socklen_t * addrlen){
    pthread_mutex_lock(&socket_table_lock);
    Node * node1 =  find_node(socket_table, fd);
    pthread_mutex_unlock(&socket_table_lock);

    if(node1 == NULL)
        return -1;
    socket_container * socket_ptr = (socket_container *)node1->container;

    char * buff = (char *)buffer;

    pthread_mutex_lock(&socket_ptr->lock);
    if(socket_ptr->receive_message == NULL){
        debug_log("waiting for receving a message\n");
        pthread_cond_wait(&socket_ptr->cond, &socket_ptr->lock);
        debug_log("Message recieved\n");
    }
    pthread_mutex_unlock(&socket_ptr->lock);

    receive_message_container * message_container = (receive_message_container *)socket_ptr->receive_message->container;

    int i =0 ;
    for(i=0 ;i < len ; i++){
        buff[i] = message_container->buffer[i];
    }

    for(i=0 ; i < 14 ; i++){
        addr->sa_data[i] = message_container->addr.sa_data[i];
    }
    addr->sa_family  = message_container->addr.sa_family;

    *addrlen = message_container->addrlen;
    int retvalue = message_container->len;

    pthread_mutex_lock(&socket_ptr->lock);
    socket_ptr->receive_message = delete_node(socket_ptr->receive_message, socket_ptr->receive_message->key);
    pthread_mutex_unlock(&socket_ptr->lock);

    return retvalue;
}

int dropMessage(float p){
    float r = (float)rand()/(float)RAND_MAX;

    if(r < p)
        return 1;
    return 0;
}

float getRatioForTransmission(int length){
    return (float)retranmission_count /(float)length;
}

rsocket.c
