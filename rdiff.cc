#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <node.h>
#include <nan.h>

#include <librsync-config.h>
#include <librsync.h>
#include "rdiff.h"

using namespace v8;

static size_t block_len = RS_DEFAULT_BLOCK_LEN;
static size_t strong_len = RS_DEFAULT_STRONG_LEN;

static int show_stats = 0;

static int bzip2_level = 0;
static int gzip_level  = 0;

enum {
  OPT_GZIP = 1069, OPT_BZIP2
};

FILE * rs_file_open(char const *filename, char const *mode)
{
  FILE * f;
  int is_write;

  is_write = mode[0] == 'w';

  if (!filename  ||  !strcmp("-", filename)) {
    if (is_write) {
      return stdout;
    }
    else {
      return stdin;
    }
  }

  if (!(f = fopen(filename, mode))) {
    exit(RS_IO_ERROR);
  }

  return f;
}

int rs_file_close (FILE * f)
{
  if ((f == stdin) || (f == stdout)) return 0;
  return fclose(f);
}


static rs_result signature(char const * in, char const * out) {
  FILE * basis_file, * sig_file;
  rs_stats_t stats;
  rs_result result;

  basis_file = rs_file_open(in, "rb");
  sig_file = rs_file_open(out, "wb");

  result = rs_sig_file(basis_file, sig_file, block_len, strong_len, &stats);

  rs_file_close(sig_file);
  rs_file_close(basis_file);

  if (result != RS_DONE) {
    return result;
  }
    
  return result;
}

static rs_result delta(char const * sig_name, char const * in, char const * out)
{
    FILE            *sig_file, *new_file, *delta_file;
    rs_result       result;
    rs_signature_t  *sumset;
    rs_stats_t      stats;

    sig_file = rs_file_open(sig_name, "rb");
    new_file = rs_file_open(in, "rb");
    delta_file = rs_file_open(out, "wb");

    result = rs_loadsig_file(sig_file, &sumset, &stats);
    if (result != RS_DONE)
        return result;

    if (show_stats) 
        rs_log_stats(&stats);

    if ((result = rs_build_hash_table(sumset)) != RS_DONE)
        return result;

    result = rs_delta_file(sumset, new_file, delta_file, &stats);

    rs_free_sumset(sumset);

    rs_file_close(delta_file);
    rs_file_close(new_file);
    rs_file_close(sig_file);

    return result;
}

static rs_result patch(char const * basis_name, char const * in, char const * out)
{
    /*  patch BASIS [DELTA [NEWFILE]] */
    FILE               *basis_file, *delta_file, *new_file;
    rs_stats_t          stats;
    rs_result           result;

    basis_file = rs_file_open(basis_name, "rb");
    delta_file = rs_file_open(in, "rb");
    new_file =   rs_file_open(out, "wb");

    result = rs_patch_file(basis_file, delta_file, new_file, &stats);

    rs_file_close(new_file);
    rs_file_close(delta_file);
    rs_file_close(basis_file);

    return result;
}

char * get(v8::Local<v8::Value> value, const char *fallback = "") {
    if (value->IsString()) {
        v8::String::AsciiValue string(value);
        char *str = (char *) malloc(string.length() + 1);
        strcpy(str, *string);
        return str;
    }
    char *str = (char *) malloc(strlen(fallback) + 1);
    strcpy(str, fallback);
    return str;
}

NAN_METHOD (GenerateSignature) {
  NanScope();
  int est = signature(get(args[0]), get(args[1]));
  NanReturnValue (Number::New(est));
}

NAN_METHOD (GenerateDelta) {
  NanScope();
  int est = delta(get(args[0]), get(args[1]), get(args[2]));
  NanReturnValue (Number::New(est));
}

NAN_METHOD (Patch) {
  NanScope();
  int est = patch(get(args[0]), get(args[1]), get(args[2]));
  NanReturnValue (Number::New(est));
}

void Init(Handle<Object> exports) {
  exports->Set(NanSymbol("signature"), FunctionTemplate::New(GenerateSignature)->GetFunction());
  exports->Set(NanSymbol("delta"), FunctionTemplate::New(GenerateDelta)->GetFunction());
  exports->Set(NanSymbol("patch"), FunctionTemplate::New(Patch)->GetFunction());
}

NODE_MODULE(rdiff, Init)

