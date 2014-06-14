# xhash - super tiny hash table [![Build Status](https://travis-ci.org/wasabiz/xhash.png)](https://travis-ci.org/wasabiz/xhash)

- written in pure C99
- all components are in single header file

# Usage

```c
xhash x;

xh_init_str(&x, sizeof(mystruct));

xh_put(&x, "aaa", mystructA);
xh_put(&x, "bbb", mystructB);
xh_put(&x, "ccc", mystructC);

xh_val(xh_get(&x, "aaa")) == mystructA;

xh_destroy(&x);
```

# License

This software is licensed under the 2-clause BSD license. See LICENSE for details.

# Version

- 2014.06.14 - Version 0.4
- 2014.03.25 - Version 0.3.4
- 2014.03.24 - Version 0.3.3
- 2014.03.20 - Version 0.3

# Auther

Yuichi Nishiwaki
