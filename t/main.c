#include <assert.h>
#include <stdio.h>

#include "../xhash.h"

void
test_iteration(int ec)
{
  xhash x;
  xh_iter it;
  int i, c = 0;

  xh_init(&x, sizeof(int), xh_ptr_hash, xh_ptr_equal);

  for (i = 0; i < ec; ++i) {
    xh_put(&x, i, &i);
  }

  xh_begin(&it, &x);
  while (xh_next(&it)) {
    ++c;
  }

  assert(c == ec);
}

void
test_resize(size_t c)
{
  xhash x;
  size_t i;

  xh_init(&x, sizeof(int), xh_ptr_hash, xh_ptr_equal);

  for (i = 0; i < c; ++i) {
    xh_put(&x, i, &i);
  }

  assert(x.count == c);
  assert(x.count <= x.size * XHASH_RESIZE_RATIO);
}

void
test()
{
  xh_entry *e;
  xhash x;
  int one = 1, two = 2, three = 3;

  xh_init(&x, sizeof(int), xh_str_hash, xh_str_equal);

  xh_put(&x, "a", &one);
  xh_put(&x, "b", &two);
  xh_put(&x, "c", &three);

  e = xh_get(&x, "a");
  assert(e && xh_val(e, int) == one);
  e = xh_get(&x, "b");
  assert(e && xh_val(e, int) == two);
  e = xh_get(&x, "c");
  assert(e && xh_val(e, int) == three);

  xh_destroy(&x);
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
