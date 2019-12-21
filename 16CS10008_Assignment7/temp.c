#include <stdio.h>
#include <stdlib.h>

struct _row {
  int a;
  int b;
  struct _row* next;
};

struct _row* initTable() {
  struct _row* table;
  table = NULL;
  return table;
}

void addRowInLast(struct _row* table, int a, int b) {
  struct _row* new_row = (struct _row*)malloc(sizeof(struct _row));
  new_row->a = a;
  new_row->b = b;
  new_row->next = NULL;

  while(table != NULL) table = table->next;
  table = new_row;
}

void addRowInFront(struct _row* table, int a, int b) {
  struct _row* new_row = (struct _row*)malloc(sizeof(struct _row));
  new_row->a = a;
  new_row->b = b;
  new_row->next = NULL;

  new_row->next = table;
  table = new_row;
}

void printTable(struct _row* table) {
  printf("start printing...\n");
  while (table != NULL) {
    printf("%d, %d\n", table->a, table->b);
    table = table->next;
  }
  printf("printing done...\n");
}

int main(int argc, char const *argv[]) {
  struct _row* table;
  table = initTable();
  int i;
  for(i=0; i<10; i++) addRowInFront(table, i , 2*i);
  printTable(table);
  return 0;
}
