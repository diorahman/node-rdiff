#include "rdiff.h"
#include <iostream>

using namespace v8;

// Signature
class GenerateSignatureWorker : public Nan::AsyncWorker {
    public:
        GenerateSignatureWorker(Nan::Callback *callback, v8::Local<v8::Value> in, v8::Local<v8::Value> out)
            : Nan::AsyncWorker(callback), in(get(in)), out(get(out))
        {}

        ~GenerateSignatureWorker() {}

        void Execute() {
            ret = signature(in.c_str(), out.c_str());
            if (ret != 0) {
                errmsg = (char *) "Error generating signature";
            }
        }

        void HandleOKCallback() {
            Nan::HandleScope scope;
            v8::Local<v8::Value> argv[] = {
                Nan::Null(),
                Nan::New<Number>(1)
            };
            callback->Call(2, argv);
        }

        void HandleErrorCallback() {
            Nan::HandleScope scope;
            v8::Local<v8::Value> argv[] = {
                Nan::Error(errmsg)
            };
            callback->Call(1, argv);
        }

    private:
        int ret;
        std::string in;
        std::string out;
        char *errmsg;
};

class GenerateDeltaWorker : public Nan::AsyncWorker {
    public:
        GenerateDeltaWorker(Nan::Callback *callback, v8::Local<v8::Value> sig_name,v8::Local<v8::Value> in, v8::Local<v8::Value> out)
            : Nan::AsyncWorker(callback), sig_name(get(sig_name)), in(get(in)), out(get(out))
        {}

        ~GenerateDeltaWorker() {}

        void Execute() {
            ret = delta(sig_name.c_str(), in.c_str(), out.c_str());
            if (ret != 0) {
                errmsg = (char *) "Error generating delta";
            }
        }

        void HandleOKCallback() {
            Nan::HandleScope scope;
            v8::Local<v8::Value> argv[] = {
                Nan::Null(),
                Nan::New<Number>(1)
            };
            callback->Call(2, argv);
        }

        void HandleErrorCallback() {
            Nan::HandleScope scope;
            v8::Local<v8::Value> argv[] = {
                Nan::Error(errmsg)
            };
            callback->Call(1, argv);
        }

    private:
        int ret;
        std::string sig_name;
        std::string in;
        std::string out;
        char *errmsg;
};

class PatchWorker : public Nan::AsyncWorker {
    public:
        PatchWorker(Nan::Callback *callback, v8::Local<v8::Value> basis_name, v8::Local<v8::Value> in, v8::Local<v8::Value> out)
            : Nan::AsyncWorker(callback), basis_name(get(basis_name)), in(get(in)), out(get(out))
        {}

        ~PatchWorker() {}

        void Execute() {
            ret = patch(basis_name.c_str(), in.c_str(), out.c_str());
            if (ret != 0) {
                errmsg = (char *) "Error patching";
            }
        }

        void HandleOKCallback() {
            Nan::HandleScope scope;
            v8::Local<v8::Value> argv[] = {
                Nan::Null(),
                Nan::New<Number>(1)
            };
            callback->Call(2, argv);
        }

        void HandleErrorCallback() {
            Nan::HandleScope scope;
            v8::Local<v8::Value> argv[] = {
                Nan::Error(errmsg)
            };
            callback->Call(1, argv);
        }

    private:
        int ret;
        std::string basis_name;
        std::string in;
        std::string out;
        char *errmsg;
};

NAN_METHOD (GenerateSignatureSync) {
    int ret = signature(get(info[0]).c_str(), get(info[1]).c_str());
    info.GetReturnValue().Set(ret);
}

NAN_METHOD(GenerateSignatureAsync) {
    Nan::Callback *callback = new Nan::Callback(info[2].As<Function>());
    Nan::AsyncQueueWorker(new GenerateSignatureWorker(callback, info[0], info[1]));
}

NAN_METHOD (GenerateDeltaSync) {
    int ret = delta(get(info[0]).c_str(), get(info[1]).c_str(), get(info[2]).c_str());
    info.GetReturnValue().Set(ret);
}

NAN_METHOD(GenerateDeltaAsync) {
    Nan::Callback *callback = new Nan::Callback(info[3].As<Function>());
    Nan::AsyncQueueWorker(new GenerateDeltaWorker(callback, info[0], info[1], info[2]));
}

NAN_METHOD (PatchSync) {
    int ret = patch(get(info[0]).c_str(), get(info[1]).c_str(), get(info[2]).c_str());
    info.GetReturnValue().Set(ret);
}

NAN_METHOD(PatchAsync) {
    Nan::Callback *callback = new Nan::Callback(info[3].As<Function>());
    Nan::AsyncQueueWorker(new PatchWorker(callback, info[0], info[1], info[2]));
}

NAN_MODULE_INIT(InitAll) {
    Nan::Set(target, Nan::New<String>("signatureSync").ToLocalChecked(),
            Nan::GetFunction(Nan::New<v8::FunctionTemplate>(GenerateSignatureSync)).ToLocalChecked());

    Nan::Set(target, Nan::New<String>("signature").ToLocalChecked(),
            Nan::GetFunction(Nan::New<v8::FunctionTemplate>(GenerateSignatureAsync)).ToLocalChecked());

    Nan::Set(target, Nan::New<String>("deltaSync").ToLocalChecked(),
            Nan::GetFunction(Nan::New<v8::FunctionTemplate>(GenerateDeltaSync)).ToLocalChecked());

    Nan::Set(target, Nan::New<String>("delta").ToLocalChecked(),
            Nan::GetFunction(Nan::New<v8::FunctionTemplate>(GenerateDeltaAsync)).ToLocalChecked());

    Nan::Set(target, Nan::New<String>("patchSync").ToLocalChecked(),
            Nan::GetFunction(Nan::New<v8::FunctionTemplate>(PatchSync)).ToLocalChecked());

    Nan::Set(target, Nan::New<String>("patch").ToLocalChecked(),
            Nan::GetFunction(Nan::New<v8::FunctionTemplate>(PatchAsync)).ToLocalChecked());
}

NODE_MODULE(rdiff, InitAll)

static size_t block_len = RS_DEFAULT_BLOCK_LEN;
static size_t strong_len = RS_DEFAULT_STRONG_LEN;

// cleaned up for a while
// static int bzip2_level = 0;
// static int gzip_level  = 0;

enum {
  OPT_GZIP = 1069, OPT_BZIP2
};

FILE * rs_file_open(const char *filename, const char *mode) {
    FILE * f;
    int is_write;

    is_write = mode[0] == 'w';

    if (!filename || !strcmp("-", filename)) {
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

int rs_file_close (FILE * f) {
    if ((f == stdin) || (f == stdout)) return 0;
    return fclose(f);
}

rs_result signature(const char *in, const char *out) {
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

rs_result delta(const char *sig_name, const char *in, const char *out) {
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

rs_result patch(const char* basis_name, const char *in, const char *out) {
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

inline std::string get(const v8::Local<v8::Value> &value) {
    return std::string(*v8::String::Utf8Value(value));
}

