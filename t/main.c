#include <assert.h>
#include <stdio.h>

#include "../xhash.h"

void
test_iteration(xhash *x, int ec)
{
  struct xh_iter it;
  int c = 0;

  for (xh_begin(x, &it); ! xh_isend(&it); xh_next(&it)) {
    ++c;
  }
  assert(c == ec);
}

void
test()
{
  struct xh_entry *e;

  xhash *x = xh_new();

  xh_put(x, "a", 1);
  xh_put(x, "b", 2);
  xh_put(x, "c", 3);

  e = xh_get(x, "a");
  assert(e && e->val == 1);
  e = xh_get(x, "b");
  assert(e && e->val == 2);
  e = xh_get(x, "c");
  assert(e && e->val == 3);

  test_iteration(x, 3);

  xh_destroy(x);
}

int
main()
{
  puts("---- xhash test started ----");

  test();

  puts("---- xhash test successfully finished ----");

  return 0;
}
