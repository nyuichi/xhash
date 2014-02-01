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

/* simple string to int hash table */

char *
strdup__(const char *s)
{
  size_t len;
  char *r;

  len = strlen(s);
  r = malloc(len + 1);
  memcpy(r, s, len);
  r[len] = '\0';

  return r;
}

#define XHASH_INIT_SIZE 11

typedef struct xh_entry {
  struct xh_entry *next;
  const char *key;
  int val;
} xh_entry;

typedef struct xhash {
  struct xh_entry **buckets;
  int size;
} xhash;

static inline struct xhash *
xh_new()
{
  struct xhash *x;

  x = (struct xhash *)malloc(sizeof(struct xhash));
  x->size = XHASH_INIT_SIZE;
  x->buckets = (struct xh_entry **)calloc(XHASH_INIT_SIZE + 1, sizeof(struct xh_entry *));
  return x;
}

static unsigned
xh_hash(const char *str)
{
  unsigned hash = 0;

  while (*str) {
    hash = hash * 31 + *str++;
  }
  return hash;
}

static inline struct xh_entry *
xh_get(struct xhash *x, const char *key)
{
  size_t idx;
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
  size_t idx;
  struct xh_entry *e;

  if ((e = xh_get(x, key))) {
    e->val = val;
    return e;
  }

  idx = xh_hash(key) % x->size;
  e = (struct xh_entry *)malloc(sizeof(struct xh_entry));
  e->next = x->buckets[idx];
  e->key = strdup__(key);
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
      free((void*)e->key);
      free(e);
      e = d;
    }
  }
  free(x);
}

typedef struct xh_iter {
  struct xh_entry *e;
  int bidx;
} xh_iter;

static inline void
xh_begin(struct xhash *x, struct xh_iter *it)
{
  int bidx;

  for (bidx = 0; bidx < x->size; ++bidx) {
    if (x->buckets[bidx])
      break;
  }
  it->e = x->buckets[bidx];
  it->bidx = bidx;
}

static inline void
xh_next(struct xhash *x, struct xh_iter *it)
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
  it->e = x->buckets[bidx];
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
