#undef NDEBUG
#include "../subset.h"
#include <assert.h>

int main() {
  Subset *s = new_subset(10);

  subset_add(s, 2);
  assert(s->size == 1);
  assert(s->size_all == 10);

  subset_add(s, 4);
  assert(s->size == 2);
  assert(s->size_all == 10);

  subset_add(s, 6);
  assert(s->size == 3);
  assert(s->size_all == 10);

  assert(!subset_contains(s, 1));
  assert(!subset_contains(s, 3));
  assert(!subset_contains(s, 0));
  assert(!subset_contains(s, 9));
  assert(subset_contains(s, 2));
  assert(subset_contains(s, 4));
  assert(subset_contains(s, 6));

  subset_remove(s, 6);
  assert(s->size == 2);
  assert(s->size_all == 10);
  assert(!subset_contains(s, 6));

  subset_remove(s, 4);
  assert(s->size == 1);
  assert(s->size_all == 10);
  assert(!subset_contains(s, 4));

  subset_remove(s, 2);
  assert(s->size == 0);
  assert(s->size_all == 10);
  assert(!subset_contains(s, 2));

  free_subset(s);
}
