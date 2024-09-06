#undef NDEBUG
#include "../sat.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  char *input = "c zzzzz\n"
                "p cnf 5 8\n"
                "-1 0\n"
                "c aaaaa\n"
                "2 0\n"
                "-3 0\n"
                "-3 -1 4 0\n"
                "c bbbb\n"
                "-4 0\n"
                "-4 -2 0\n"
                "5 0\n"
                "5 1 0\n";

  char *expected = "p cnf 5 8\n"
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

  char *output;
  size_t size;
  FILE *ostream = open_memstream(&output, &size);
  assert(ostream);
  write_cnf(cnf, ostream);
  fflush(ostream);

  assert(strcmp(expected, output) == 0);

  fclose(ostream);
  free(output);
  free_cnf(cnf);
  return 0;
}
