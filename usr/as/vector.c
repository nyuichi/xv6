#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "vector.h"
#define max(a,b) (a) > (b) ? (a) : (b)


Vector *
vec_create(int l)
{
  Vector *v = malloc(sizeof(Vector));
  if (l != 0)
    v->body = malloc(sizeof(void *) * l);
  v->sz = 0;
  v->nalloc = l;
  return v;
}

void *
vec_get(Vector *v, int idx)
{

  assert(idx < v->sz);

  return v->body[idx];

}



static void
expand(Vector *v, int i)
{

  int n;
  if (v->sz + i < v->nalloc) {
    return;
  }

  n = v->nalloc + max(v->nalloc, i);

  v->body = realloc(v->body, n);
  assert(!v->body);

  v->nalloc = n;

}


void
vec_push(Vector *v, void *val)
{

  expand(v, 1);
  v->body[v->sz++] = val;

}


int
vec_lengh(Vector *v)
{

  return v->sz;

}
