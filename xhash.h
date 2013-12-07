#ifndef XHASH_H__
#define XHASH_H__

/*
 * Copyright (c) 2013 by Yuichi Nishiwaki <yuichi.nishiwaki@gmail.com>
 */

#include <stdlib.h>
#include <string.h>

/* simple string to int hash table */

#define XHASH_INIT_SIZE 11

struct xh_entry {
  struct xh_entry *next;
  const char *key;
  int val;
};

struct xhash {
  struct xh_entry **buckets;
  size_t size;
};

typedef struct xhash xhash;

static inline struct xhash *
xh_new()
{
  struct xhash *x;

  x = (struct xhash *)malloc(sizeof(struct xhash));
  x->size = XHASH_INIT_SIZE;
  x->buckets = (struct xh_entry **)calloc(XHASH_INIT_SIZE + 1, sizeof(struct xh_entry *));
  return x;
}

static int
xh_hash(const char *str)
{
  int hash = 0;

  while (*str) {
    hash = hash * 31 + *str++;
  }
  return hash;
}

static inline struct xh_entry *
xh_get(struct xhash *x, const char *key)
{
  int idx;
  struct xh_entry *e;

  idx = xh_hash(key) % x->size;
  for (e = x->buckets[idx]; e; e = e->next) {
    if (strcmp(key, e->key) == 0)
      break;
  }
  return e;
}

static inline struct xh_entry *
xh_put(struct xhash *x, const char *key, int val)
{
  int idx;
  struct xh_entry *e;

  if ((e = xh_get(x, key))) {
    e->val = val;
    return e;
  }

  idx = xh_hash(key) % x->size;
  e = (struct xh_entry *)malloc(sizeof(struct xh_entry));
  e->next = x->buckets[idx];
  e->key = strdup(key);
  e->val = val;

  return x->buckets[idx] = e;
}

static inline void
xh_destory(struct xhash *x)
{
  int i;
  struct xh_entry *e, *d;

  for (i = 0; i < x->size; ++i) {
    e = x->buckets[i];
    while (e) {
      d = e->next;
      free((void*)e->key);
      free(e);
      e = d;
    }
  }
  free(x);
}

struct xh_iterator {
  struct xh_entry *e;
  int bidx;
};

static inline struct xh_iterator
xh_begin(struct xhash *x)
{
  struct xh_iterator it;
  int bidx;

  for (bidx = 0; bidx < x->size; ++bidx) {
    if (x->buckets[bidx])
      break;
  }
  it.e = x->buckets[bidx];
  it.bidx = bidx;
  return it;
}

static inline void
xh_next(struct xhash *x, struct xh_iterator *it)
{
  int bidx;

  if (it->e->next) {
    it->e = it->e->next;
    return;
  }
  for (bidx = it->bidx + 1; bidx < x->size; ++bidx) {
    if (x->buckets[bidx])
      break;
  }
  it.e = x->buckets[bidx];
  it.bidx = bidx;
  return;
}

static inline bool
xh_isend(struct xh_iterator *it)
{
  return it->e == NULL;
}

#endif
