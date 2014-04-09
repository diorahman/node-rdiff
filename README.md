## rdiff simple binding in node.js

http://linux.die.net/man/1/rdiff

## pre

install librsync

## example

```js
// a new file contains "hello"
var a = "a.txt";

// a file contains "hello world"
var b = "b.txt";

// `a` wants to be `b`

rdiff.signature (a, a + ".sig");
rdiff.delta (a + ".sig", b, a + ".delta");

// the `a.patched` contains "hello world"
rdiff.patch (a, a + ".delta", a + ".patched");
```

## license
MIT