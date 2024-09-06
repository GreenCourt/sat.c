#include "sat.h"
#include "subset.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
  int size;
  int size_undetermined;
  int *list; // len(list) == size
  int *pos;  // len(pos) == number_of_all_possible_literals
  int count_true;
} Clause;

static int n, m;
static Clause *clauses;
static int **clauses_related_to_var;
static int *n_clauses_related_to_var;

static Subset *unsatisfied_unit_clauses;
static Subset *unsatisfied_clauses;

static int *assign;

typedef struct {
  int *list;
  int size;
} Implication;

static void assert_clause(int clause_index) {
  Clause *c = &clauses[clause_index];
  if (c->count_true == 0) {
    assert(subset_contains(unsatisfied_clauses, clause_index));
    if (c->size_undetermined == 1)
      assert(subset_contains(unsatisfied_unit_clauses, clause_index));
    else
      assert(!subset_contains(unsatisfied_unit_clauses, clause_index));
  } else {
    assert(!subset_contains(unsatisfied_clauses, clause_index));
    assert(!subset_contains(unsatisfied_unit_clauses, clause_index));
  }
}

static void hide_literal(Clause *c, int lit) {
  assert(c->size_undetermined > 0);
  assert(c->pos[lit] < c->size_undetermined);
  int l2 = c->list[c->size_undetermined - 1];
  c->list[c->size_undetermined - 1] = lit;
  c->list[c->pos[lit]] = l2;
  c->pos[l2] = c->pos[lit];
  c->pos[lit] = c->size_undetermined - 1;
  c->size_undetermined--;
}

static void unhide_literal(Clause *c, int lit) {
  assert(c->pos[lit] >= c->size_undetermined);
  int l2 = c->list[c->size_undetermined];
  c->list[c->size_undetermined] = lit;
  c->list[c->pos[lit]] = l2;
  c->pos[l2] = c->pos[lit];
  c->pos[lit] = c->size_undetermined;
  c->size_undetermined++;
}

static bool apply_literal_to_clause(int clause_index, int lit) {
  /*
   * return true iff UNSAT
   */
  Clause *c = &clauses[clause_index];
  int inv = INVERTED(lit);
  assert(c->pos[lit] != -1 || c->pos[inv] != -1);
  assert_clause(clause_index);

  if (c->pos[lit] != -1) {
    hide_literal(c, lit);
    c->count_true++;

    if (c->count_true == 1) {
      subset_remove(unsatisfied_clauses, clause_index);
      if (c->size_undetermined == 0)
        subset_remove(unsatisfied_unit_clauses, clause_index);
    }
  }

  if (c->pos[inv] != -1) {
    hide_literal(c, inv);

    if (c->count_true == 0) {
      if (c->size_undetermined == 1)
        subset_add(unsatisfied_unit_clauses, clause_index);
      else if (c->size_undetermined == 0)
        subset_remove(unsatisfied_unit_clauses, clause_index);
    }

    if (c->size_undetermined == 0 && c->count_true == 0)
      return true;
  }

  assert_clause(clause_index);
  return false;
}

static void unapply_literal_to_clause(int clause_index, int lit) {
  Clause *c = &clauses[clause_index];
  int inv = INVERTED(lit);
  assert(c->pos[lit] != -1 || c->pos[inv] != -1);
  assert_clause(clause_index);

  if (c->pos[lit] != -1) {
    unhide_literal(c, lit);
    c->count_true--;

    if (c->count_true == 0) {
      subset_add(unsatisfied_clauses, clause_index);
      if (c->size_undetermined == 1)
        subset_add(unsatisfied_unit_clauses, clause_index);
    }
  }

  if (c->pos[inv] != -1) {
    unhide_literal(c, inv);

    if (c->count_true == 0) {
      if (c->size_undetermined == 1)
        subset_add(unsatisfied_unit_clauses, clause_index);
      else if (c->size_undetermined == 2)
        subset_remove(unsatisfied_unit_clauses, clause_index);
    }
  }
  assert_clause(clause_index);
}

static bool apply_literal_to_all(int lit) {
  /*
   * return true iff UNSAT
   */
  int var = VAR(lit);
  assert(assign[var] == 0);
  assign[var] = LITERAL_SIGN(lit);
  bool unsat = false;
  int sz = n_clauses_related_to_var[var];
  int *affected_clause_indices = clauses_related_to_var[var];
  int i;
  for (i = 0; i < sz; ++i) {
    if (apply_literal_to_clause(affected_clause_indices[i], lit)) {
      unsat = true;
      break;
    }
  }
  if (unsat) {
    assign[var] = 0;
    for (int j = i; j >= 0; --j)
      unapply_literal_to_clause(affected_clause_indices[j], lit);
  }
  return unsat;
}

void unapply_literal_to_all(int lit) {
  int var = VAR(lit);
  assert(assign[var] != 0);
  assign[var] = 0;
  int sz = n_clauses_related_to_var[var];
  int *affected_clause_indices = clauses_related_to_var[var];
  for (int j = 0; j < sz; ++j)
    unapply_literal_to_clause(affected_clause_indices[j], lit);
}

Implication *new_implication() {
  Implication *imp = malloc(sizeof(Implication));
  imp->list = malloc(n * sizeof(int));
  imp->size = 0;
  return imp;
}

void free_implication(Implication *imp) {
  assert(imp);
  if (imp->list)
    free(imp->list);
  free(imp);
}

static Implication *unit_propagation() {
  /*
   * return Implication* if SAT else NULL
   */
  Implication *imp = new_implication();

  while (unsatisfied_unit_clauses->size) {
    int idx =
        unsatisfied_unit_clauses->list[unsatisfied_unit_clauses->size - 1];
    Clause *c = &clauses[idx];
    assert(c->size_undetermined == 1);
    int lit = c->list[0];

    if (apply_literal_to_all(lit)) {
      for (int i = 0; i < imp->size; ++i) {
        int lit_imp = imp->list[i];
        unapply_literal_to_all(lit_imp);
      }
      free_implication(imp);
      return NULL;
    }

    imp->list[imp->size++] = lit;
  }

  return imp;
}

static void check_sat(Cnf *cnf) {
  assert(unsatisfied_clauses->size == 0);
  for (int i = 0; i < cnf->m; ++i) {
    bool ok = cnf->clause_size[i] == 0;
    for (int j = 0; j < cnf->clause_size[i]; ++j) {
      int x = cnf->clause[i][j];
      if (assign[ABS(x) - 1] * ABS(x) == x) {
        ok = true;
        break;
      }
    }
    assert(ok);
    (void)ok;
  }
}

static int next_literal() {
  assert(unsatisfied_clauses->size > 0);
  Clause *c = &clauses[unsatisfied_clauses->list[0]];
  assert(c->size_undetermined > 0);
  assert(assign[VAR(c->list[0])] == 0);
  return c->list[0];
}

static void rec(int lit) {
  assert(lit >= 0);
  assert(lit < 2 * n);

  apply_literal_to_all(lit);

  Implication *imp = unit_propagation();

  if (imp == NULL) {
    unapply_literal_to_all(lit);
    return;
  }

  if (unsatisfied_clauses->size == 0) {
    free_implication(imp);
    return;
  }

  int lit2 = next_literal();
  rec(lit2);

  if (unsatisfied_clauses->size == 0) {
    free_implication(imp);
    return;
  }

  rec(INVERTED(lit2));

  if (unsatisfied_clauses->size == 0) {
    free_implication(imp);
    return;
  }

  for (int i = 0; i < imp->size; ++i) {
    int lit_imp = imp->list[i];
    unapply_literal_to_all(lit_imp);
  }
  free_implication(imp);
  unapply_literal_to_all(lit);
}

static bool init(Cnf *cnf) {
  /*
   * return true iff UNSAT
   */
  n = cnf->n;
  m = cnf->m;
  assign = calloc(n, sizeof(int));

  {
    // initialize clauses
    clauses = malloc(m * sizeof(Clause));
    for (int i = 0; i < m; i++) {
      Clause *c = &clauses[i];
      int size = cnf->clause_size[i];
      c->size = size;
      c->size_undetermined = size;
      c->count_true = 0;
      c->list = malloc(size * sizeof(int));
      c->pos = malloc(n * 2 * sizeof(int));
      for (int j = 0; j < n * 2; ++j)
        c->pos[j] = -1;

      int idx = 0;
      for (int j = 0; j < size; ++j) {
        int lit = LITERAL(ABS(cnf->clause[i][j]) - 1, cnf->clause[i][j]);
        assert(lit < 2 * n);
        if (c->pos[lit] != -1)
          continue;
        c->list[idx] = lit;
        c->pos[lit] = idx;
        idx++;
      }
      assert(idx == size);
    }
  }

  {
    n_clauses_related_to_var = calloc(n, sizeof(int));
    int *buf = calloc(n, sizeof(int));
    for (int i = 0; i < m; ++i) {
      Clause *c = &clauses[i];
      int size = c->size;
      for (int j = 0; j < size; ++j) {
        int var = VAR(c->list[j]);
        if (buf[var] != i + 1)
          n_clauses_related_to_var[var]++;
        buf[var] = i + 1;
      }
    }
    free(buf);
  }

  {
    clauses_related_to_var = malloc(n * sizeof(int *));
    for (int v = 0; v < n; ++v)
      clauses_related_to_var[v] =
          malloc(n_clauses_related_to_var[v] * sizeof(int));
    int *buf = calloc(n, sizeof(int));
    int *cnt = calloc(n, sizeof(int));
    for (int i = 0; i < m; ++i) {
      Clause *c = &clauses[i];
      int size = c->size;
      for (int j = 0; j < size; ++j) {
        int var = VAR(c->list[j]);
        if (buf[var] != i + 1)
          clauses_related_to_var[var][cnt[var]++] = i;
        buf[var] = i + 1;
      }
    }
    for (int v = 0; v < n; ++v)
      assert(cnt[v] == n_clauses_related_to_var[v]);
    free(buf);
    free(cnt);
  }

  {
    unsatisfied_clauses = new_subset(m);
    unsatisfied_unit_clauses = new_subset(m);
    for (int i = 0; i < m; ++i) {
      Clause *c = &clauses[i];
      if (c->size_undetermined > 0)
        subset_add(unsatisfied_clauses, i);
      if (c->size_undetermined == 1)
        subset_add(unsatisfied_unit_clauses, i);
    }
  }

  Implication *imp = unit_propagation();
  if (imp == NULL)
    return true;

  free_implication(imp);
  return false;
}

static void cleanup() {
  for (int i = 0; i < m; i++) {
    free(clauses[i].list);
    free(clauses[i].pos);
  }
  free(clauses);

  for (int v = 0; v < n; ++v)
    free(clauses_related_to_var[v]);
  free(clauses_related_to_var);
  free(n_clauses_related_to_var);

  free_subset(unsatisfied_clauses);
  free_subset(unsatisfied_unit_clauses);
}

// reterns variable assignment if SAT,
// NULL if UNSAT
int *solve(Cnf *cnf) {
  if (cnf->n == 0 && cnf->m == 0)
    return malloc(sizeof(int));

  if (cnf->n == 0)
    return NULL;

  bool unsat = init(cnf);

  if (unsat) {
    cleanup();
    free(assign);
    return NULL;
  }

  if (unsatisfied_clauses->size != 0) {
    int lit = next_literal();
    rec(lit);
    if (unsatisfied_clauses->size != 0) {
      for (int i = 0; i < n; ++i)
        assert(assign[i] == 0);
      rec(INVERTED(lit));
    }
  }

  if (unsatisfied_clauses->size == 0) {
    check_sat(cnf);
    for (int i = 0; i < n; ++i)
      if (assign[i] == 0)
        assign[i] = -1; // arbitrary assignment
    cleanup();
    return assign;
  }

  free(assign);
  cleanup();
  return NULL;
}
