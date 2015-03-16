#ifndef VECTOR_H
#define VECTOR_H
typedef struct {
  void **body;
  int sz;
  int nalloc;
} Vector;
Vector *vec_create(int );
void   *vec_get(Vector *, int);
void    vec_push(Vector *, void *);
int     vec_lengh(Vector *);
void    vec_free(Vector *);
#endif
