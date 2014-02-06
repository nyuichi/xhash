#ifndef XHASH_H__
#define XHASH_H__

/*
 * Copyright (c) 2013 by Yuichi Nishiwaki <yuichi.nishiwaki@gmail.com>
 */

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* simple object to int hash table */

#define XHASH_INIT_SIZE 11

typedef struct xh_entry {
  struct xh_entry *next;
  const void *key;
  int val;
} xh_entry;

typedef int (*xh_hashf)(const void *);
typedef int (*xh_equalf)(const void *, const void *);

typedef struct xhash {
  struct xh_entry **buckets;
  int size;
  xh_hashf hashf;
  xh_equalf equalf;
} xhash;

static inline struct xhash *
xh_new(xh_hashf hashf, xh_equalf equalf)
{
  struct xhash *x;

  x = (struct xhash *)malloc(sizeof(struct xhash));
  x->size = XHASH_INIT_SIZE;
  x->buckets = (struct xh_entry **)calloc(XHASH_INIT_SIZE + 1, sizeof(struct xh_entry *));
  x->hashf = hashf;
  x->equalf = equalf;
  return x;
}

static inline struct xh_entry *
xh_get(struct xhash *x, const void *key)
{
  size_t idx;
  struct xh_entry *e;

  idx = ((unsigned)x->hashf(key)) % x->size;
  for (e = x->buckets[idx]; e; e = e->next) {
    if (x->equalf(key, e->key))
      break;
  }
  return e;
}

static inline struct xh_entry *
xh_put(struct xhash *x, const void *key, int val)
{
  size_t idx;
  struct xh_entry *e;

  if ((e = xh_get(x, key))) {
    e->val = val;
    return e;
  }

  idx = ((unsigned)x->hashf(key)) % x->size;
  e = (struct xh_entry *)malloc(sizeof(struct xh_entry));
  e->next = x->buckets[idx];
  e->key = key;
  e->val = val;

  return x->buckets[idx] = e;
}

static inline void
xh_destroy(struct xhash *x)
{
  int i;
  struct xh_entry *e, *d;

  for (i = 0; i < x->size; ++i) {
    e = x->buckets[i];
    while (e) {
      d = e->next;
      free(e);
      e = d;
    }
  }
  free(x);
}

typedef struct xh_iter {
  struct xhash *x;
  struct xh_entry *e;
  int bidx;
} xh_iter;

static inline void
xh_begin(struct xhash *x, struct xh_iter *it)
{
  int bidx;

  it->x = x;

  for (bidx = 0; bidx < x->size; ++bidx) {
    if (x->buckets[bidx])
      break;
  }
  it->e = x->buckets[bidx];
  it->bidx = bidx;
}

static inline void
xh_next(struct xh_iter *it)
{
  int bidx;

  if (it->e->next) {
    it->e = it->e->next;
    return;
  }
  for (bidx = it->bidx + 1; bidx < it->x->size; ++bidx) {
    if (it->x->buckets[bidx])
      break;
  }
  it->e = it->x->buckets[bidx];
  it->bidx = bidx;
  return;
}

static inline bool
xh_isend(struct xh_iter *it)
{
  return it->e == NULL;
}

#if defined(__cplusplus)
}
#endif

#endif
