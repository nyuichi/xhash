# xhash - super tiny hash table [![Build Status](https://travis-ci.org/wasabiz/xhash.png)](https://travis-ci.org/wasabiz/xhash)

- written in pure C99
- all components are in single header file

# Usage

```c
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
```

# License

This software is licensed under the 2-clause BSD license. See LICENSE for details.

# Version

- 2014.03.24 - Version 0.3.3
- 2014.03.20 - Version 0.3

# Auther

Yuichi Nishiwaki
