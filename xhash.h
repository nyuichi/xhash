#ifndef XHASH_H
#define XHASH_H

/*
 * Copyright (c) 2013-2014 by Yuichi Nishiwaki <yuichi.nishiwaki@gmail.com>
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

#define XHASH_INIT_SIZE 11
#define XHASH_RESIZE_RATIO 0.75

#define XHASH_ALIGNMENT 3       /* quad word alignment */
#define XHASH_MASK (~((1 << XHASH_ALIGNMENT) - 1))
#define XHASH_ALIGN(i) ((((i) - 1) & XHASH_MASK) + (1 << XHASH_ALIGNMENT))

typedef struct xh_entry {
  struct xh_entry *next;
  int hash;
  struct xh_entry *fw, *bw;
  const char *key;              /* == val + XHASH_ALIGN(vwidth) */
  char val[];
} xh_entry;

#define xh_key(e,type) (*(type *)((e)->key))
#define xh_val(e,type) (*(type *)((e)->val))

typedef int (*xh_hashf)(const void *, void *);
typedef int (*xh_equalf)(const void *, const void *, void *);

typedef struct xhash {
  xh_entry **buckets;
  size_t size, count, kwidth, vwidth;
  xh_hashf hashf;
  xh_equalf equalf;
  xh_entry *chain;
  void *data;
} xhash;

/** Protected Methods:
 * static inline void xh_init_(xhash *x, size_t, size_t, xh_hashf, xh_equalf, void *);
 * static inline xh_entry *xh_get_(xhash *x, const void *key);
 * static inline xh_entry *xh_put_(xhash *x, const void *key, void *val);
 * static inline void xh_del_(xhash *x, const void *key);
 */

/* string map */
static inline void xh_init_str(xhash *x, size_t width);
static inline xh_entry *xh_get_str(xhash *x, const char *key);
static inline xh_entry *xh_put_str(xhash *x, const char *key, void *);
static inline void xh_del_str(xhash *x, const char *key);

/* object map */
static inline void xh_init_ptr(xhash *x, size_t width);
static inline xh_entry *xh_get_ptr(xhash *x, const void *key);
static inline xh_entry *xh_put_ptr(xhash *x, const void *key, void *);
static inline void xh_del_ptr(xhash *x, const void *key);

/* int map */
static inline void xh_init_int(xhash *x, size_t width);
static inline xh_entry *xh_get_int(xhash *x, int key);
static inline xh_entry *xh_put_int(xhash *x, int key, void *);
static inline void xh_del_int(xhash *x, int key);

static inline size_t xh_size(xhash *x);
static inline void xh_clear(xhash *x);
static inline void xh_destroy(xhash *x);

static inline xh_entry *xh_begin(xhash *x);
static inline xh_entry *xh_next(xh_entry *e);


static inline void
xh_bucket_realloc(xhash *x, size_t newsize)
{
  x->size = newsize;
  x->buckets = realloc(x->buckets, (x->size + 1) * sizeof(xh_entry *));
  memset(x->buckets, 0, (x->size + 1) * sizeof(xh_entry *));
}

static inline void
xh_init_(xhash *x, size_t kwidth, size_t vwidth, xh_hashf hashf, xh_equalf equalf, void *data)
{
  x->size = 0;
  x->buckets = NULL;
  x->count = 0;
  x->kwidth = kwidth;
  x->vwidth = vwidth;
  x->hashf = hashf;
  x->equalf = equalf;
  x->chain = NULL;
  x->data = data;

  xh_bucket_realloc(x, XHASH_INIT_SIZE);
}

static inline xh_entry *
xh_get_(xhash *x, const void *key)
{
  int hash;
  size_t idx;
  xh_entry *e;

  hash = x->hashf(key, x->data);
  idx = ((unsigned)hash) % x->size;
  for (e = x->buckets[idx]; e; e = e->next) {
    if (e->hash == hash && x->equalf(key, e->key, x->data))
      break;
  }
  return e;
}

static inline void
xh_resize_(xhash *x, size_t newsize)
{
  xhash y;
  xh_entry *it;
  size_t idx;

  xh_init_(&y, x->kwidth, x->vwidth, x->hashf, x->equalf, x->data);
  xh_bucket_realloc(&y, newsize);

  for (it = xh_begin(x); it != NULL; it = xh_next(it)) {
    idx = ((unsigned)it->hash) % y.size;
    /* reuse entry object */
    it->next = y.buckets[idx];
    y.buckets[idx] = it;
    y.count++;
  }

  y.chain = x->chain;

  free(x->buckets);

  /* copy all members from y to x */
  memcpy(x, &y, sizeof(xhash));
}

static inline xh_entry *
xh_put_(xhash *x, const void *key, void *val)
{
  int hash;
  size_t idx;
  xh_entry *e;

  if ((e = xh_get_(x, key))) {
    memcpy(e->val, val, x->vwidth);
    return e;
  }

  if (x->count + 1 > x->size * XHASH_RESIZE_RATIO) {
    xh_resize_(x, x->size * 2 + 1);
  }

  hash = x->hashf(key, x->data);
  idx = ((unsigned)hash) % x->size;
  e = (xh_entry *)malloc(offsetof(xh_entry, val) + XHASH_ALIGN(x->vwidth) + x->kwidth);
  e->next = x->buckets[idx];
  e->hash = hash;
  e->key = e->val + XHASH_ALIGN(x->vwidth);
  memcpy((void *)e->key, key, x->kwidth);
  memcpy(e->val, val, x->vwidth);

  if (x->chain == NULL) {
    x->chain = e;
    e->fw = e->bw = NULL;
  } else {
    x->chain->fw = e;
    e->bw = x->chain;
    e->fw = NULL;
    x->chain = e;
  }

  x->count++;

  return x->buckets[idx] = e;
}

static inline void
xh_del_(xhash *x, const void *key)
{
  int hash;
  size_t idx;
  xh_entry *e, *d;

  hash = x->hashf(key, x->data);
  idx = ((unsigned)hash) % x->size;
  if (x->buckets[idx]->hash == hash && x->equalf(key, x->buckets[idx]->key, x->data)) {
    e = x->buckets[idx]->next;
    if (e->fw) {
      e->fw->bw = e->bw;
    }
    if (e->bw) {
      e->bw->fw = e->fw;
    }
    if (x->chain == e) {
      x->chain = e->bw;
    }
    free(x->buckets[idx]);
    x->buckets[idx] = e;
  }
  else {
    for (e = x->buckets[idx]; ; e = e->next) {
      if (e->next->hash == hash && x->equalf(key, e->next->key, x->data))
        break;
    }
    d = e->next->next;
    if (e->next->fw) {
      e->next->fw->bw = e->next->bw;
    }
    if (e->next->bw) {
      e->next->bw->fw = e->next->fw;
    }
    if (x->chain == e->next) {
      x->chain = e->next->bw;
    }
    free(e->next);
    e->next = d;
  }

  x->count--;
}

static inline size_t
xh_size(xhash *x)
{
  return x->count;
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

  x->chain = NULL;
  x->count = 0;
}

static inline void
xh_destroy(xhash *x)
{
  xh_clear(x);
  free(x->buckets);
}

/* string map */

static inline int
xh_str_hash(const void *key, void *data)
{
  const char *str = *(const char **)key;
  int hash = 0;

  (void)data;

  while (*str) {
    hash = hash * 31 + *str++;
  }
  return hash;
}

static inline int
xh_str_equal(const void *key1, const void *key2, void *data)
{
  (void)data;

  return strcmp(*(const char **)key1, *(const char **)key2) == 0;
}

static inline void
xh_init_str(xhash *x, size_t width)
{
  xh_init_(x, sizeof(const char *), width, xh_str_hash, xh_str_equal, NULL);
}

static inline xh_entry *
xh_get_str(xhash *x, const char *key)
{
  return xh_get_(x, &key);
}

static inline xh_entry *
xh_put_str(xhash *x, const char *key, void *val)
{
  return xh_put_(x, &key, val);
}

static inline void
xh_del_str(xhash *x, const char *key)
{
  xh_del_(x, &key);
}

/* object map */

static inline int
xh_ptr_hash(const void *key, void *data)
{
  (void)data;

  return (size_t)*(const void **)key;
}

static inline int
xh_ptr_equal(const void *key1, const void *key2, void *data)
{
  (void) data;

  return *(const void **)key1 == *(const void **)key2;
}

static inline void
xh_init_ptr(xhash *x, size_t width)
{
  xh_init_(x, sizeof(const void *), width, xh_ptr_hash, xh_ptr_equal, NULL);
}

static inline xh_entry *
xh_get_ptr(xhash *x, const void *key)
{
  return xh_get_(x, &key);
}

static inline xh_entry *
xh_put_ptr(xhash *x, const void *key, void *val)
{
  return xh_put_(x, &key, val);
}

static inline void
xh_del_ptr(xhash *x, const void *key)
{
  xh_del_(x, &key);
}

/* int map */

static inline int
xh_int_hash(const void *key, void *data)
{
  (void)data;

  return *(int *)key;
}

static inline int
xh_int_equal(const void *key1, const void *key2, void *data)
{
  (void)data;

  return *(int *)key1 == *(int *)key2;
}

static inline void
xh_init_int(xhash *x, size_t width)
{
  xh_init_(x, sizeof(int), width, xh_int_hash, xh_int_equal, NULL);
}

static inline xh_entry *
xh_get_int(xhash *x, int key)
{
  return xh_get_(x, &key);
}

static inline xh_entry *
xh_put_int(xhash *x, int key, void *val)
{
  return xh_put_(x, &key, val);
}

static inline void
xh_del_int(xhash *x, int key)
{
  xh_del_(x, &key);
}

/** iteration */

static inline xh_entry *
xh_begin(xhash *x)
{
  return x->chain;
}

static inline xh_entry *
xh_next(xh_entry *e)
{
  return e->bw;
}

#if defined(__cplusplus)
}
#endif

#endif
