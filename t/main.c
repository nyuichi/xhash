#include <assert.h>
#include <stdio.h>

#include "../xhash.h"

void
test_iteration(int ec)
{
  xhash *x;
  xh_iter it;
  int i, c = 0;

  x = xh_new_int();

  for (i = 0; i < ec; ++i) {
    xh_put_int(x, i, i);
  }

  for (xh_begin(x, &it); ! xh_isend(&it); xh_next(&it)) {
    ++c;
  }
  assert(c == ec);
}

void
test_resize(size_t c)
{
  xhash *x;
  size_t i;

  x = xh_new_int();

  for (i = 0; i < c; ++i) {
    xh_put_int(x, i, i);
  }

  assert(x->count == c);
  assert(x->count <= x->size * XHASH_RESIZE_RATIO);
}

void
test()
{
  xh_entry *e;

  xhash *x = xh_new_str();

  xh_put(x, "a", 1);
  xh_put(x, "b", 2);
  xh_put(x, "c", 3);

  e = xh_get(x, "a");
  assert(e && e->val == 1);
  e = xh_get(x, "b");
  assert(e && e->val == 2);
  e = xh_get(x, "c");
  assert(e && e->val == 3);

  xh_destroy(x);
}

int
main()
{
  puts("---- xhash test started ----");

  test();
  test_iteration(30);
  test_resize(300);

  puts("---- xhash test successfully finished ----");

  return 0;
}
