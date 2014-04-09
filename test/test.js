var test = require("tap").test;
var rdiff = require ("../");
var fs = require ("fs");

var a = __dirname + "/a.txt";
var b = __dirname + "b.txt";

function remove (file){
  if (fs.existsSync(file)) {
    fs.unlinkSync(file);
  }
}

remove(a);
remove(b);
remove(a + ".sig");
remove(a + ".patched");

test("three steps to patch a file", function (t) {
  fs.writeFileSync(a, "hello");
  fs.writeFileSync(b, "hello world");

  rdiff.signature (a, a + ".sig");
  rdiff.delta (a + ".sig", b, a + ".delta");
  rdiff.patch (a, a + ".delta", a + ".patched");

  var patched = fs.readFileSync(a + ".patched");

  t.equal(patched.toString(), "hello world", "should patched as 'hello world'");
  t.end();
});
