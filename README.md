# xhash - super tiny hash table [![Build Status](https://travis-ci.org/wasabiz/xhash.png)](https://travis-ci.org/wasabiz/xhash)

- written in pure C99
- all components are in single header file

# Usage

```c
xhash x;

xh_init_str(&x, sizeof(mystruct));

xh_put_str(&x, "aaa", mystructA);
xh_put_str(&x, "bbb", mystructB);
xh_put_str(&x, "ccc", mystructC);

xh_val(xh_get(&x, "aaa"), mystruct) == mystructA;

xh_destroy(&x);
```

# License

This software is licensed under the 2-clause BSD license. See LICENSE for details.

# Version

- 2014.09.16 - Version 0.5

    The implementation switches to ordered hash map (hash map with doubly-linked list).
    Accordingly, iteration interface changed.

- 2014.08.01 - Version 0.4.1
- 2014.06.14 - Version 0.4
- 2014.03.25 - Version 0.3.4
- 2014.03.24 - Version 0.3.3
- 2014.03.20 - Version 0.3

# Auther

Yuichi Nishiwaki
