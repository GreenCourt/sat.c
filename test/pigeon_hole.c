#undef NDEBUG
#include "../sat.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int variable_ph(int pigeon, int hole, int n_hole) {
  return pigeon * n_hole + hole + 1;
}

Cnf *pigeon_hole(int n_pigeion, int n_hole) {
  assert(n_pigeion > 0 && n_hole > 0);
  int n = n_pigeion * n_hole;
  int m = n_pigeion + n_pigeion * (n_pigeion - 1) / 2 * n_hole;
  int **clause = malloc(m * sizeof(int *));
  int *clause_size = malloc(m * sizeof(int));

  int clause_index = 0;

  // each pigion must have a hole.
  for (int p = 0; p < n_pigeion; ++p) {
    clause_size[clause_index] = n_hole;
    clause[clause_index] = malloc(n_hole * sizeof(int));
    for (int h = 0; h < n_hole; ++h)
      clause[clause_index][h] = variable_ph(p, h, n_hole);
    clause_index++;
  }

  // Each hole cannot be shared.
  for (int p1 = 0; p1 < n_pigeion; ++p1) {
    for (int p2 = p1 + 1; p2 < n_pigeion; ++p2) {
      for (int h = 0; h < n_hole; ++h) {
        clause_size[clause_index] = 2;
        clause[clause_index] = malloc(2 * sizeof(int));
        clause[clause_index][0] = -variable_ph(p1, h, n_hole);
        clause[clause_index][1] = -variable_ph(p2, h, n_hole);
        clause_index++;
      }
    }
  }

  assert(clause_index == m);
  Cnf *cnf = malloc(sizeof(Cnf));
  cnf->n = n;
  cnf->m = m;
  cnf->clause = clause;
  cnf->clause_size = clause_size;
  return cnf;
}

int main() {
  for (int p = 1; p <= 9; p++) {
    for (int h = 1; h <= 9; h++) {
      printf("\033[2K\rp:%d h:%d", p, h);
      fflush(stdout);
      Cnf *cnf = pigeon_hole(p, h);
      int *assign = solve(cnf);
      bool sat = assign != NULL;

      if (assign)
        free(assign);

      if (sat == (p <= h)) {
        free_cnf(cnf);
        continue;
      }

      printf("\n");
      fprintf(stderr, "failed! pigeon:%d hole:%d\n", p, h);
      FILE *fp = fopen("failed.cnf", "w");
      assert(fp);
      fprintf(fp, "c pigeon:%d hole:%d\n", p, h);
      write_cnf(cnf, fp);
      fclose(fp);
      free_cnf(cnf);
      exit(1);
    }
  }
  printf("\033[2K\r");
  return 0;
}
