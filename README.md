# xhash - super tiny hash table [![Build Status](https://travis-ci.org/wasabiz/xhash.png)](https://travis-ci.org/wasabiz/xhash)

- written in pure C99
- all components are in single header file

# Usage

```c
typedef struct xh_entry {
  struct xh_entry *next;
  int hash;
  const void *key;
  long val;
} xh_entry;

typedef int (*xh_hashf)(const void *);
typedef int (*xh_equalf)(const void *, const void *);

typedef struct xhash {
  xh_entry **buckets;
  size_t size, count;
  xh_hashf hashf;
  xh_equalf equalf;
} xhash;

static inline void xh_init(xhash *x, xh_hashf hashf, xh_equalf equalf);
static inline xh_entry *xh_get(xhash *x, const void *key);
static inline xh_entry *xh_put(xhash *x, const void *key, long val);
static inline void xh_del(xhash *x, const void *key);
static inline void xh_clear(xhash *x);
static inline void xh_destroy(xhash *x);

static int xh_str_hash(const void *key);
static int xh_str_equal(const void *key1, const void *key2);
static int xh_ptr_hash(const void *key);
static int xh_ptr_equal(const void *key1, const void *key2);

typedef struct xh_iter {
  xhash *x;
  xh_entry *e;
  size_t bidx;
} xh_iter;

static inline void xh_begin(xh_iter *it, xhash *x);
static inline void xh_next(xh_iter *it);
static inline int xh_end(xh_iter *it);
```

# License

This software is licensed under the 2-clause BSD license. See LICENSE for details.

# Auther

Yuichi Nishiwaki
