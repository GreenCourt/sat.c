#include <stdbool.h>
#include <stdio.h>

typedef struct {
  int n; // number of variables
  int m; // number of clauses
  int **clause;
  int *clause_size;
} Cnf;

void free_cnf(Cnf *cnf);
Cnf *read_cnf(FILE *fp);
bool write_cnf(Cnf *cnf, FILE *fp);

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define LITERAL(x, sign) (2 * (x) + ((sign) < 0))
#define INVERTED(lit) (IS_NEGATIVE(lit) ? lit - 1 : lit + 1)
#define IS_POSITIVE(lit) (~(lit) & 1)
#define IS_NEGATIVE(lit) ((lit) & 1)
#define LITERAL_SIGN(lit) (IS_NEGATIVE(lit) ? -1 : 1)
#define VAR(lit) ((lit) / 2)

int *solve(Cnf *cnf);
