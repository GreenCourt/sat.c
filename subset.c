#include "subset.h"
#include <assert.h>
#include <stdlib.h>

#define SUBSET_NOT_CONTAINED -1

Subset *new_subset(int size) {
  Subset *s = malloc(sizeof(Subset));
  s->size_all = size;
  s->size = 0;
  s->list = malloc(size * sizeof(int));
  int *pos = s->pos = malloc(size * sizeof(int));
  for (int i = 0; i < size; ++i)
    pos[i] = SUBSET_NOT_CONTAINED;
  return s;
}

void free_subset(Subset *s) {
  assert(s);
  free(s->list);
  free(s->pos);
  free(s);
}

bool subset_contains(Subset *s, int x) {
  return s->pos[x] != SUBSET_NOT_CONTAINED;
}

void subset_add(Subset *s, int x) {
  assert(x < s->size_all);
  if (s->pos[x] != SUBSET_NOT_CONTAINED)
    return;
  int size = s->size;
  assert(size < s->size_all);
  s->list[size] = x;
  s->pos[x] = size;
  s->size++;
}

void subset_remove(Subset *s, int x) {
  assert(s->pos[x] != SUBSET_NOT_CONTAINED);
  assert(s->size > 0);
  int p = s->pos[x];
  s->list[p] = s->list[s->size - 1];
  s->pos[s->list[p]] = p;
  s->pos[x] = SUBSET_NOT_CONTAINED;
  s->size--;
}
