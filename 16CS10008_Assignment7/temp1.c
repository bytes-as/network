#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>


// // TO BE REVIED AGAIN AS ALWAYS <-- NOT WORKING...
// char* intToChar(char *char_id, int id) {
//   // char char_id[3];
//   int i=0;
//   while (id) {
//     char_id[2-i] = (id%10) + '0';
//     id = id/10;
//     i++;
//   }
//   return char_id;
// }
//
// int main(int argc, char const *argv[]) {
//   char id[3];
//   intToChar(id,513);
//   // strrev(a, i);
//   printf("input = %s\n", id);
//   return 0;
// }

int charToInt(char *buffer, int *id) {
  int temp = (char)buffer[3] - '0';
  printf("--> %d\n", temp);
  *id += buffer[3] - '0';
  *id += (buffer[2] - '0') * 10;
  *id += (buffer[3] - '0') * 10;
  return *id;
}

int chartoInt(char *buffer) {
  int id = 0;
  id += buffer[3] - '0';
  id += (buffer[2] - '0') * 10;
  id += (buffer[3] - '0') * 10;
  return id;
}

void createAck(char *ack_msg, char *id) {
  // char *ack_msg;
  // ack_msg = (char *)malloc(5 * sizeof(char));
  memset(ack_msg, '\0', 5*sizeof(char));
  ack_msg[0] = 'A';
  strcat(ack_msg, id);
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

int main(int argc, char const *argv[]) {
  int a = 123;
  char *ack = (char *)malloc(3 * sizeof(char));
  // createAck(ack, a);
  intToChar(ack, a);
  printf("integer = %s  ", ack);
  return 0;
}

// struct node {
//   int data;
//   struct node* next;
// };
//
// struct node* head;
//
// void init() {
//   head = NULL;
// }
//
//
// void append(int k) {
//   struct node* n = (struct node*)malloc(sizeof(struct node));
//   n->data = k;
//   n->next = NULL;
//   n->next = head;
//   head = n;
//   return ;
// }
//
// void push(int k) {
//   struct node* n = (struct node*)malloc(sizeof(struct node));
//   n->data = k;
//   n->next = NULL;
//   if (head == NULL) {
//     head = n;
//     return ;
//   }
//   struct node* temp = head;
//   while(temp->next != NULL) temp = temp->next;
//   temp->next = n;
//   return;
// }
//
// void printT()  {
//   struct node* temp = head;
//   while(temp != NULL) {
//     printf("%d  ", temp->data);
//     temp = temp->next;
//   }
//   printf("\n");
//   return ;
// }
//
// void delete(int k) {
//   struct node* temp = head;
//   if(temp->data == k){
//     head = temp->next;
//     free(temp);
//   }
//   int count = 0;
//   while(temp != NULL && temp->data != k) {
//     temp = temp->next;
//     count++;
//   }
//   if(temp == NULL) return ;
//   int i;
//   temp = head;
//   for(i=0; i<count-1; i++) temp = temp->next;
//   struct node* t;
//   t = temp->next;
//   temp->next = t->next;
//   free(t);
// }
// int main(int argc, char const *argv[]) {
//   int i;
//   init();
//   for(i=0; i<10; i++) {
//     append(i);
//     push(2*i);
//   }
//   printT();
//   delete(14);
//   delete(5);
//   delete(9);
//   printT();
//   return 0;
// }
