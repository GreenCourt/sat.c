#include <stdbool.h>

typedef struct {
  int size_all;
  int size;
  int *list;
  int *pos;
} Subset;

Subset *new_subset(int size);
void free_subset(Subset *s);
bool subset_contains(Subset *s, int x);
void subset_add(Subset *s, int x);
void subset_remove(Subset *s, int x);
