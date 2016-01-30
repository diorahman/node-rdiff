#ifndef RDIFF_H
#define RDIFF_H

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <node.h>
#include <nan.h>

#include <string>

#include <librsync-config.h>
#include <librsync.h>

inline std::string get(const v8::Local<v8::Value> &value);

extern "C" {
  rs_result signature(char const *in, char const *out);
  rs_result delta(char const *sig_name, char const *in, char const *out);
  rs_result patch(char const *sig_name, char const *in, char const *out);
}

#endif
