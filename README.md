## rdiff simple binding in node.js

http://linux.die.net/man/1/rdiff

## pre

install librsync `~0.9.7` (since the stable release is this one [http://sourceforge.net/projects/librsync/](stable))

## example

the sync api

```js
// a new file contains "hello"
var a = "a.txt";

// a file contains "hello world"
var b = "b.txt";

// `a` wants to be `b`

rdiff.signatureSync (a, a + ".sig");
rdiff.deltaSync (a + ".sig", b, a + ".delta");

// the `a.patched` contains "hello world"
rdiff.patchSync (a, a + ".delta", a + ".patched");
```

do the same with async api

```js
rdiff.signature(a, a + ".sig", function (err){
  rdiff.delta(a + ".sig", b, a + ".delta", function(err){
    rdiff.patch (a, a + ".delta", a + ".patched", function(err) {
      // yeah! contents of `a` should equal contents of `a.patched`
    });
  })
});
```

## warning

it doesn't do file existence checking yet.

## license
MIT
