#include "sat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  if (argc == 1) {
    fprintf(stderr, "arguments required\n");
    return 2;
  }
  char *file_name = argv[1];

  FILE *fp = strcmp(file_name, "-") == 0 ? stdin : fopen(file_name, "r");
  if (fp == NULL) {
    fprintf(stderr, "faild to open \"%s\"\n", file_name);
    return 1;
  }

  Cnf *cnf = read_cnf(fp);

  if (strcmp(file_name, "-") != 0)
    fclose(fp);

  if (cnf == NULL)
    return 1;

  int *assign = solve(cnf);

  if (assign) {
    int n = cnf->n;
    puts("SAT");
    for (int i = 0; i < n; ++i)
      printf("%d ", assign[i] * (i + 1));
    printf("0\n");
    free(assign);
  } else {
    puts("UNSAT");
  }

  free_cnf(cnf);
  return 0;
}
