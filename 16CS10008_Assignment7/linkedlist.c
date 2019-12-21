#include <stdio.h>
#include <stdlib.h>

struct node {
  int key;
  struct node* next;
};

struct node*head;

void initTable() {
  head = NULL;
  // return head;
}

void printTable(struct node* head) {
  while (head != NULL) {
    printf("%d  ", head->key);
    head = head->next;
  }
  printf("\n");
}

void append(struct node** head_ref, int k) {
  struct node* new_node;
  new_node = (struct node*)malloc(sizeof(struct node));
  new_node->key = k;
  new_node->next = *head_ref;
  *head_ref = new_node;
}

void push(struct node** head_ref, int k) {
  struct node* temp = *head_ref;
  struct node* new_node;
  new_node = (struct node*)malloc(sizeof(struct node));
  new_node->key = k;
  new_node->next = NULL;
  if(*head_ref == NULL) {
    *head_ref = new_node;
    return ;
  }
  while(temp->next != NULL) temp = temp->next;
  temp->next = new_node;
}

int main(int argc, char const *argv[]) {
  int i;
  struct node* table = head;
  initTable();
  for(i=0; i<10; i++) {
    push(&table, i);
    append(&table, 2*i);
  }
  struct node* temp = table;
  while (temp->next != NULL) {
    temp = temp->next;
  }
  temp->key = 1000;
  printTable(table);
  return 0;
}
