var rdiff = require ("../");
var fs = require ("fs");

var a = __dirname + "/a.txt";
var b = __dirname + "/b.txt";

function remove (file){
    if (fs.existsSync(file)) {
        fs.unlinkSync(file);
    }
}

function pre () {
    remove(a);
    remove(b);
    remove(a + ".sig");
    remove(a + ".patched");

    fs.writeFileSync(a, "hello");
    fs.writeFileSync(b, "hello world");
}

describe("three steps patching", function() {

    beforeEach(pre);

    it("should patch the file using sync api", function(done) {
        rdiff.signatureSync (a, a + ".sig");
        rdiff.deltaSync (a + ".sig", b, a + ".delta");
        rdiff.patchSync (a, a + ".delta", a + ".patched");

        var patched = fs.readFileSync(a + ".patched");
        patched.toString().should.equal("hello world");

        done();
    });

    it ("should patch the file using async api", function(done){
        rdiff.signature (a, a + ".sig", function (err, ret) {
            rdiff.delta (a + ".sig", b, a + ".delta", function (err, ret) {
                rdiff.patch (a, a + ".delta", a + ".patched", function (err, ret) {

                    var patched = fs.readFileSync(a + ".patched");
                    patched.toString().should.equal("hello world");

                    done();
                });
            });
        });
    });
});
