#include "rdiff.h"

using namespace v8;

class GenerateSignatureWorker : public NanAsyncWorker {
 public:
  GenerateSignatureWorker(NanCallback *callback, v8::Local<v8::Value> in, v8::Local<v8::Value> out)
    : NanAsyncWorker(callback), in(get(in)), out(get(out)) {}
  ~GenerateSignatureWorker() {}
  void Execute () {
    ret = signature(in, out);
    if (ret != 0) {
      errmsg = (char *) "Error generating signature";
    }
  }

  void HandleOKCallback () {
    NanScope();
    Local<Value> argv[] = {
      NanNull(),
      NanNew<Number>(ret)
    };
    callback->Call(2, argv);
  };

  void HandleErrorCallback () {
    NanScope();
    Local<Value> argv[] = {
      Exception::Error(NanNew<String>(errmsg))
    };
    callback->Call(1, argv);
  }

 private:
  int ret;
  const char * in;
  const char * out;
  char * errmsg;
};

class GenerateDeltaWorker : public NanAsyncWorker {
 public:
  GenerateDeltaWorker(NanCallback *callback, v8::Local<v8::Value> sig_name, v8::Local<v8::Value> in, v8::Local<v8::Value> out)
    : NanAsyncWorker(callback), sig_name(get(sig_name)), in(get(in)), out(get(out)) {}
  ~GenerateDeltaWorker() {}

  void Execute () {
    ret = delta(sig_name, in, out);
    if (ret != 0) {
      errmsg = (char*) "Error generating delta";
    }
  }

  void HandleOKCallback () {
    NanScope();
    Local<Value> argv[] = {
      NanNull(),
      NanNew<Number>(ret)
    };
    callback->Call(2, argv);
  };

  void HandleErrorCallback (){
    NanScope();
    Local<Value> argv[] = {
      Exception::Error(NanNew<String>(errmsg))
    };
    callback->Call(1, argv);
  }

 private:
  int ret;
  const char * sig_name;
  const char * in;
  const char * out;
  char * errmsg;
};

class PatchWorker : public NanAsyncWorker {
 public:
  PatchWorker(NanCallback *callback, v8::Local<v8::Value> basis_name, v8::Local<v8::Value> in, v8::Local<v8::Value> out)
    : NanAsyncWorker(callback), basis_name(get(basis_name)), in(get(in)), out(get(out)) {}
  ~PatchWorker() {}
  
  void Execute () {
    ret = patch(basis_name, in, out);
    if (ret != 0) {
      errmsg = (char*) "Error patching";
    }
  }
  
  void HandleOKCallback () {
    NanScope();
    Local<Value> argv[] = {
      NanNull(),
      NanNew<Number>(ret)
    };
    callback->Call(2, argv);
  };

  void HandleErrorCallback (){
    NanScope();
    Local<Value> argv[] = {
      Exception::Error(NanNew<String>(errmsg))
    };
    callback->Call(1, argv);
  }

 private:
  int ret;
  const char * basis_name;
  const char * in;
  const char * out;
  char * errmsg;
};

NAN_METHOD (GenerateSignatureSync) {
  NanScope();
  int ret = signature(get(args[0]), get(args[1]));
  NanReturnValue (NanNew<Number>(ret));
}

NAN_METHOD(GenerateSignatureAsync) {
  NanScope();
  NanCallback *callback = new NanCallback(args[2].As<Function>());
  NanAsyncQueueWorker(new GenerateSignatureWorker(callback, args[0], args[1]));
  NanReturnUndefined();
}

NAN_METHOD (GenerateDeltaSync) {
  NanScope();
  int ret = delta(get(args[0]), get(args[1]), get(args[2]));
  NanReturnValue (NanNew<Number>(ret));
}

NAN_METHOD(GenerateDeltaAsync) {
  NanScope();
  NanCallback * callback = new NanCallback(args[3].As<Function>());
  NanAsyncQueueWorker(new GenerateDeltaWorker(callback, args[0], args[1], args[2]));
  NanReturnUndefined();
}

NAN_METHOD (PatchSync) {
  NanScope();
  int ret = patch(get(args[0]), get(args[1]), get(args[2]));
  NanReturnValue (NanNew<Number>(ret));
}

NAN_METHOD(PatchAsync) {
  NanScope();
  NanCallback * callback = new NanCallback(args[3].As<Function>());
  NanAsyncQueueWorker(new PatchWorker(callback, args[0], args[1], args[2]));
  NanReturnUndefined();
}


void Init(Handle<Object> exports) {
  exports->Set(NanNew<String>("signature"), NanNew<FunctionTemplate>(GenerateSignatureAsync)->GetFunction());
  exports->Set(NanNew<String>("signatureSync"), NanNew<FunctionTemplate>(GenerateSignatureSync)->GetFunction());
  exports->Set(NanNew<String>("delta"), NanNew<FunctionTemplate>(GenerateDeltaAsync)->GetFunction());
  exports->Set(NanNew<String>("deltaSync"), NanNew<FunctionTemplate>(GenerateDeltaSync)->GetFunction());
  exports->Set(NanNew<String>("patch"), NanNew<FunctionTemplate>(PatchAsync)->GetFunction());
  exports->Set(NanNew<String>("patchSync"), NanNew<FunctionTemplate>(PatchSync)->GetFunction());
}

static size_t block_len = RS_DEFAULT_BLOCK_LEN;
static size_t strong_len = RS_DEFAULT_STRONG_LEN;

// cleaned up for a while
// static int bzip2_level = 0;
// static int gzip_level  = 0;

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

rs_result signature(char const * in, char const * out) {
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

rs_result delta(char const * sig_name, char const * in, char const * out)
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

  if ((result = rs_build_hash_table(sumset)) != RS_DONE)
      return result;

  result = rs_delta_file(sumset, new_file, delta_file, &stats);

  rs_free_sumset(sumset);

  rs_file_close(delta_file);
  rs_file_close(new_file);
  rs_file_close(sig_file);

  return result;
}

rs_result patch(char const * basis_name, char const * in, char const * out)
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

char * get(v8::Local<v8::Value> value, const char * fallback) {
  if (value->IsString()) {
      NanAsciiString string(value);
      char *str = (char *) malloc(string.length() + 1);
      strcpy(str, *string);
      return str;
  }
  char *str = (char *) malloc(strlen(fallback) + 1);
  strcpy(str, fallback);
  return str;
}

NODE_MODULE(rdiff, Init)

