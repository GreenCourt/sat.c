#undef NDEBUG
#include "../sat.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  assert(LITERAL(0, 1) == 0);
  assert(LITERAL(0, -1) == 1);
  assert(LITERAL(1, 1) == 2);
  assert(LITERAL(1, -1) == 3);
  assert(LITERAL(2, 1) == 4);
  assert(LITERAL(2, -1) == 5);

  assert(!IS_POSITIVE(1));
  assert(IS_NEGATIVE(1));
  assert(IS_POSITIVE(2));
  assert(!IS_NEGATIVE(2));

  assert(VAR(LITERAL(0, -1)) == 0);
  assert(VAR(LITERAL(0, 1)) == 0);
  assert(VAR(LITERAL(1, -1)) == 1);
  assert(VAR(LITERAL(1, 1)) == 1);
  assert(VAR(LITERAL(2, -1)) == 2);
  assert(VAR(LITERAL(2, 1)) == 2);

  assert(INVERTED(LITERAL(2, 1)) == LITERAL(2, -1));
  assert(INVERTED(LITERAL(2, -1)) == LITERAL(2, 1));

  char *input = "p cnf 5 8\n"
                "-1 0\n"
                "2 0\n"
                "-3 0\n"
                "-3 -1 4 0\n"
                "-4 0\n"
                "-4 -2 0\n"
                "5 0\n"
                "5 1 0\n";

  FILE *istream = fmemopen(input, strlen(input), "r");
  assert(istream);
  Cnf *cnf = read_cnf(istream);
  assert(cnf);
  fclose(istream);

  int *assign = solve(cnf);
  assert(assign);
  assert(assign[0] == -1);
  assert(assign[1] == 1);
  assert(assign[2] == -1);
  assert(assign[3] == -1);
  assert(assign[4] == 1);
  free(assign);

  free_cnf(cnf);
  return 0;
}
