#include "sat.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void free_cnf(Cnf *cnf) {
  assert(cnf);
  int m = cnf->m;
  for (int i = 0; i < m; ++i)
    free(cnf->clause[i]);
  free(cnf->clause);
  free(cnf->clause_size);
  free(cnf);
}

Cnf *read_cnf(FILE *fp) {
  assert(fp);
  int nline = 0;
  char *line = NULL;
  size_t bufer_length = 0;
  ssize_t nread;

  Cnf *cnf = malloc(sizeof(Cnf));
  int n = -1;
  int m = -1;
  int **clause = NULL;
  int *clause_size = NULL;

  int clause_index = 0;

  while ((nread = getline(&line, &bufer_length, fp)) != -1) {
    nline++;
    if (nread == 0 || line[0] == 'c')
      continue;

    if (line[0] == 'p') {
      if (n != -1) {
        fprintf(stderr, "input file error at line %d\n", nline);
        for (int i = 0; i < clause_index; ++i)
          free(clause[i]);
        free(clause_size);
        free(clause);
        free(cnf);
        if (line)
          free(line);
        return NULL;
      }

      int s = sscanf(line, "p cnf %d %d", &n, &m);
      if (s != 2 || n < 0 || m < 0) {
        fprintf(stderr, "input file error at line %d\n", nline);
        free(cnf);
        if (line)
          free(line);
        return NULL;
      }
      cnf->n = n;
      cnf->m = m;
      cnf->clause = clause = malloc(m * sizeof(int *));
      cnf->clause_size = clause_size = calloc(m, sizeof(int));
      continue;
    }

    if (clause_index >= m) {
      fprintf(stderr, "too many clauses\n");
      free_cnf(cnf);
      if (line)
        free(line);
      return NULL;
    }

    if (n == -1) {
      fprintf(stderr, "input file error at line %d\n", nline);
      free(cnf);
      if (line)
        free(line);
      return NULL;
    }

    {
      // check clause_size before malloc for clause[clause_index]
      char *buf = malloc((strlen(line) + 1) * sizeof(char));
      strcpy(buf, line);
      char *p = strtok(buf, " ");
      do {
        int x;
        sscanf(p, "%d", &x);
        if (x == 0)
          break;
        clause_size[clause_index]++;
      } while ((p = strtok(NULL, " ")) != NULL);
      free(buf);
    }

    {
      clause[clause_index] = malloc(clause_size[clause_index] * sizeof(int));
      int cnt = 0;
      char *p = strtok(line, " ");
      do {
        int x;
        sscanf(p, "%d", &x);
        if (ABS(x) > n) {
          fprintf(stderr, "invalid literal %d\n", x);
          for (int i = 0; i <= clause_index; ++i)
            free(clause[i]);
          free(clause_size);
          free(clause);
          free(cnf);
          return NULL;
        }
        if (x == 0)
          break;
        clause[clause_index][cnt++] = x;
      } while ((p = strtok(NULL, " ")) != NULL);
      assert(cnt == clause_size[clause_index]);
    }

    clause_index++;
  }

  if (line)
    free(line);

  if (clause_index != m) {
    fprintf(stderr, "invalid number of clauses\n");
    free_cnf(cnf);
    return NULL;
  }

  return cnf;
}

bool write_cnf(Cnf *cnf, FILE *fp) {
  assert(cnf);
  assert(fp);
  fprintf(fp, "p cnf %d %d\n", cnf->n, cnf->m);
  int m = cnf->m;
  int **clause = cnf->clause;
  for (int i = 0; i < m; ++i) {
    int sz = cnf->clause_size[i];
    for (int j = 0; j < sz; ++j)
      fprintf(fp, "%d ", clause[i][j]);
    fprintf(fp, "0\n");
  }
  return false;
}
