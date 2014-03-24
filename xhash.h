#ifndef XHASH_H__
#define XHASH_H__

/*
 * Copyright (c) 2013 by Yuichi Nishiwaki <yuichi.nishiwaki@gmail.com>
 */

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

/* simple object to object hash table */

/* xhash is potentially a weak map; it does not retain the ownership of keys */

#define XHASH_INIT_SIZE 11
#define XHASH_RESIZE_RATIO 0.75

typedef intmax_t xh_key_t;

typedef struct xh_entry xh_entry;

#define xh_key(e,type) ((type)(e))
#define xh_val(e,type) (*(type *)((e)->val))

typedef int (*xh_hashf)(xh_key_t);
typedef int (*xh_equalf)(xh_key_t, xh_key_t);

typedef struct xhash {
  xh_entry **buckets;
  size_t size, count, width;
  xh_hashf hashf;
  xh_equalf equalf;
} xhash;

static inline void xh_init(xhash *x, size_t width, xh_hashf hashf, xh_equalf equalf);
static inline void xh_init_str(xhash *x, size_t width);
static inline void xh_init_ptr(xhash *x, size_t width);
static inline void xh_init_int(xhash *x, size_t width); /* applicable to integer of any width */
#define xh_get(x, /* void* or int */key) xh_get_((x), (xh_key_t)(key))
#define xh_put(x, /* void* or int */key, val) xh_put_((x), (xh_key_t)(key), val)
static inline void xh_del(xhash *x, xh_key_t key);
static inline void xh_clear(xhash *x);
static inline void xh_destroy(xhash *x);

typedef struct xh_iter {
  xhash *x;
  xh_entry *e, *next;
  size_t bidx;
} xh_iter;

static inline void xh_begin(xh_iter *it, xhash *x);
static inline int xh_next(xh_iter *it);

/* implementations below */

struct xh_entry {
  struct xh_entry *next;
  int hash;
  xh_key_t key;                 /* void* or any immediate integer type */
  char val[];
};

static inline void
xh_bucket_realloc(xhash *x, size_t newsize)
{
  x->size = newsize;
  x->buckets = realloc(x->buckets, (x->size + 1) * sizeof(xh_entry *));
  memset(x->buckets, 0, (x->size + 1) * sizeof(xh_entry *));
}

static inline void
xh_init(xhash *x, size_t width, xh_hashf hashf, xh_equalf equalf)
{
  x->size = 0;
  x->buckets = NULL;
  x->count = 0;
  x->width = width;
  x->hashf = hashf;
  x->equalf = equalf;

  xh_bucket_realloc(x, XHASH_INIT_SIZE);
}

static inline int
xh_str_hash(xh_key_t key)
{
  const char *str = (const char *)key;
  int hash = 0;

  while (*str) {
    hash = hash * 31 + *str++;
  }
  return hash;
}

static inline int
xh_str_equal(xh_key_t key1, xh_key_t key2)
{
  return strcmp((const char *)key1, (const char *)key2) == 0;
}

static inline void
xh_init_str(xhash *x, size_t width)
{
  xh_init(x, width, xh_str_hash, xh_str_equal);
}

static inline int
xh_ptr_hash(xh_key_t key)
{
  return (int)key;
}

static inline int
xh_ptr_equal(xh_key_t key1, xh_key_t key2)
{
  return key1 == key2;
}

static inline void
xh_init_ptr(xhash *x, size_t width)
{
  xh_init(x, width, xh_ptr_hash, xh_ptr_equal);
}

static inline int
xh_int_hash(xh_key_t key)
{
  return (int)key;
}

static inline int
xh_int_equal(xh_key_t key1, xh_key_t key2)
{
  return key1 == key2;
}

static inline void
xh_init_int(xhash *x, size_t width)
{
  xh_init(x, width, xh_int_hash, xh_int_equal);
}

static inline xh_entry *
xh_get_(xhash *x, xh_key_t key)
{
  int hash;
  size_t idx;
  xh_entry *e;

  hash = x->hashf(key);
  idx = ((unsigned)hash) % x->size;
  for (e = x->buckets[idx]; e; e = e->next) {
    if (e->hash == hash && x->equalf(key, e->key))
      break;
  }
  return e;
}

static inline void
xh_resize(xhash *x, size_t newsize)
{
  xhash y;
  xh_iter it;
  size_t idx;

  xh_init(&y, x->width, x->hashf, x->equalf);
  xh_bucket_realloc(&y, newsize);

  xh_begin(&it, x);
  while (xh_next(&it)) {
    idx = ((unsigned)it.e->hash) % y.size;
    /* reuse entry object */
    it.e->next = y.buckets[idx];
    y.buckets[idx] = it.e;
    y.count++;
  }

  free(x->buckets);

  /* copy all members from y to x */
  memcpy(x, &y, sizeof(xhash));
}

static inline xh_entry *
xh_put_(xhash *x, xh_key_t key, void *val)
{
  int hash;
  size_t idx;
  xh_entry *e;

  if ((e = xh_get_(x, key))) {
    memcpy(e->val, val, x->width);
    return e;
  }

  if (x->count + 1 > x->size * XHASH_RESIZE_RATIO) {
    xh_resize(x, x->size * 2 + 1);
  }

  hash = x->hashf(key);
  idx = ((unsigned)hash) % x->size;
  e = (xh_entry *)malloc(offsetof(xh_entry, val) + x->width);
  e->next = x->buckets[idx];
  e->hash = hash;
  e->key = key;
  memcpy(e->val, val, x->width);

  x->count++;

  return x->buckets[idx] = e;
}

static inline void
xh_del(xhash *x, xh_key_t key)
{
  int hash;
  size_t idx;
  xh_entry *e, *d;

  hash = x->hashf(key);
  idx = ((unsigned)hash) % x->size;
  if (x->buckets[idx]->hash == hash && x->equalf(key, x->buckets[idx]->key)) {
    e = x->buckets[idx]->next;
    free(x->buckets[idx]);
    x->buckets[idx] = e;
  }
  else {
    for (e = x->buckets[idx]; ; e = e->next) {
      if (e->next->hash == hash && x->equalf(key, e->next->key))
        break;
    }
    d = e->next->next;
    free(e->next);
    e->next = d;
  }

  x->count--;
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

  x->count = 0;
}

static inline void
xh_destroy(xhash *x)
{
  xh_clear(x);
  free(x->buckets);
}

/** type specific */

/** iteration */

static inline void
xh_begin(xh_iter *it, xhash *x)
{
  size_t bidx;

  it->x = x;

  for (bidx = 0; bidx < x->size; ++bidx) {
    if (x->buckets[bidx])
      break;
  }
  it->e = NULL;
  it->next = x->buckets[bidx];
  it->bidx = bidx;
}

static inline int
xh_next(xh_iter *it)
{
  size_t bidx;

  if (! it->next) {
    return 0;
  }

  it->e = it->next;
  if (it->next->next) {
    it->next = it->next->next;
    return 1;
  }
  for (bidx = it->bidx + 1; bidx < it->x->size; ++bidx) {
    if (it->x->buckets[bidx])
      break;
  }
  it->next = it->x->buckets[bidx];
  it->bidx = bidx;
  return 1;
}

#if defined(__cplusplus)
}
#endif

#endif
