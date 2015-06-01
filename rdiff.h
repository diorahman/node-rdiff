#ifndef RDIFF_H
#define RDIFF_H

#include <ctype.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <node.h>
#include <nan.h>

#include <librsync-config.h>
#include <librsync.h>

char * get(v8::Local<v8::Value> value, const char *fallback = "");

extern "C" {
  rs_result signature(char const * in, char const * out);
  rs_result delta(char const * sig_name, char const * in, char const * out);
  rs_result patch(char const * sig_name, char const * in, char const * out);
}

#endif
