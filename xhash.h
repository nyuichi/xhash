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
#include <assert.h>

/* simple object to long hash table */

#define XHASH_INIT_SIZE 11

typedef struct xh_entry {
  struct xh_entry *next;
  const void *key;
  long val;
} xh_entry;

typedef int (*xh_hashf)(const void *);
typedef int (*xh_equalf)(const void *, const void *);

typedef struct xhash {
  xh_entry **buckets;
  size_t size;
  xh_hashf hashf;
  xh_equalf equalf;
} xhash;

static inline xhash *
xh_new(xh_hashf hashf, xh_equalf equalf)
{
  xhash *x;

  x = (xhash *)malloc(sizeof(xhash));
  x->size = XHASH_INIT_SIZE;
  x->buckets = (xh_entry **)calloc(XHASH_INIT_SIZE + 1, sizeof(xh_entry *));
  x->hashf = hashf;
  x->equalf = equalf;
  return x;
}

static int
xh_str_hash(const void *key)
{
  const char *str = key;
  int hash = 0;

  while (*str) {
    hash = hash * 31 + *str++;
  }
  return hash;
}

static int
xh_str_equal(const void *key1, const void *key2)
{
  return strcmp((const char *)key1, (const char *)key2) == 0;
}

static inline xhash *
xh_new_str()
{
  return xh_new(xh_str_hash, xh_str_equal);
}

static int
xh_ptr_hash(const void *key)
{
  return (int)(long)key;
}

static int
xh_ptr_equal(const void *key1, const void *key2)
{
  return key1 == key2;
}

static inline xhash *
xh_new_ptr()
{
  return xh_new(xh_ptr_hash, xh_ptr_equal);
}

static inline xhash *
xh_new_int()
{
  assert(sizeof(long) <= sizeof(void *));
  return xh_new(xh_ptr_hash, xh_ptr_equal);
}

static inline xh_entry *
xh_get(xhash *x, const void *key)
{
  size_t idx;
  xh_entry *e;

  idx = ((unsigned)x->hashf(key)) % x->size;
  for (e = x->buckets[idx]; e; e = e->next) {
    if (x->equalf(key, e->key))
      break;
  }
  return e;
}

static inline xh_entry *
xh_put(xhash *x, const void *key, long val)
{
  size_t idx;
  xh_entry *e;

  if ((e = xh_get(x, key))) {
    e->val = val;
    return e;
  }

  idx = ((unsigned)x->hashf(key)) % x->size;
  e = (xh_entry *)malloc(sizeof(xh_entry));
  e->next = x->buckets[idx];
  e->key = key;
  e->val = val;

  return x->buckets[idx] = e;
}

static inline void
xh_del(xhash *x, const void *key)
{
  size_t idx;
  xh_entry *e, *d;

  idx = ((unsigned)x->hashf(key)) % x->size;
  if (x->equalf(key, x->buckets[idx]->key)) {
    e = x->buckets[idx]->next;
    free(x->buckets[idx]);
    x->buckets[idx] = e;
  }
  else {
    for (e = x->buckets[idx]; ; e = e->next) {
      if (x->equalf(key, e->next->key))
        break;
    }
    d = e->next->next;
    free(e->next);
    e->next = d;
  }
}

static inline xh_entry *
xh_get_int(xhash *x, long key)
{
  return xh_get(x, (void *)key);
}

static inline xh_entry *
xh_put_int(xhash *x, long key, long val)
{
  return xh_put(x, (void *)key, val);
}

static inline void
xh_clear(xhash *x)
{
  size_t i;
  xh_entry *e, *d;

  for (i = 0; i < x->size; ++i) {
    e = x->buckets[i];
    while (e) {
      d = e->next;
      free(e);
      e = d;
    }
    x->buckets[i] = NULL;
  }
}

static inline void
xh_destroy(xhash *x)
{
  xh_clear(x);
  free(x->buckets);
  free(x);
}

typedef struct xh_iter {
  xhash *x;
  xh_entry *e;
  size_t bidx;
} xh_iter;

static inline void
xh_begin(xhash *x, xh_iter *it)
{
  size_t bidx;

  it->x = x;

  for (bidx = 0; bidx < x->size; ++bidx) {
    if (x->buckets[bidx])
      break;
  }
  it->e = x->buckets[bidx];
  it->bidx = bidx;
}

static inline void
xh_next(xh_iter *it)
{
  size_t bidx;

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

static inline int
xh_isend(xh_iter *it)
{
  return it->e == NULL;
}

#if defined(__cplusplus)
}
#endif

#endif
