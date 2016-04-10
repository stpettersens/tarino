/*
  tarino: a tar implementation with Node.js.
  Copyright 2016 Sam Saint-Pettersen.

  Native add-on.

  Dual licensed under the GPL and MIT licenses;
  see GPL-LICENSE and MIT-LICENSE respectively.
*/

#include <nan.h>
#include <string>
#include "tarino.h"

void export_write_tar_entry(const Nan::FunctionCallbackInfo<v8::Value>& info) {

    if (info.Length() < 5) {
        Nan::ThrowTypeError("tarino-native: Wrong number of arguments");
        return;
    }

    if (!info[0]->IsString() || !info[1]->IsString() 
    || !info[2]->IsNumber() || !info[3]->IsNumber() || !info[4]->IsNumber()) {
        Nan::ThrowTypeError("tarino-native: Arguments should be string, string, number (int), number (int)");
        return;
    }

    v8::String::Utf8Value tarname(info[0]->ToString());
    v8::String::Utf8Value filename(info[1]->ToString());
    int size = (int)info[2]->NumberValue();
    int modified = (int)info[3]->NumberValue();
    int etype = (int)info[4]->NumberValue();
    v8::Local<v8::Number> code = Nan::New(
    write_tar_entry(std::string(*tarname), std::string(*filename), size, modified, etype));

    info.GetReturnValue().Set(code);
}

void export_write_tar_entries(const Nan::FunctionCallbackInfo<v8::Value>& info) {

    if (info.Length() < 2) {
        Nan::ThrowTypeError("tarino_native: Wrong number of arguments");
        return;
    }

    if (!info[0]->IsString() || !info[1]->IsString()) {
        Nan::ThrowTypeError("tarino-native: Arguments should be string, string");
        return;
    }

    v8::String::Utf8Value tarname(info[0]->ToString());
    v8::String::Utf8Value manifest(info[1]->ToString());
    v8::Local<v8::Number> code = Nan::New(
    write_tar_entries(std::string(*tarname), std::string(*manifest)));

    info.GetReturnValue().Set(code);
}

void init(v8::Local<v8::Object> exports) {
    exports->Set(Nan::New("write_tar_entry").ToLocalChecked(),
    Nan::New<v8::FunctionTemplate>(export_write_tar_entry)->GetFunction());

    exports->Set(Nan::New("write_tar_entries").ToLocalChecked(),
    Nan::New<v8::FunctionTemplate>(export_write_tar_entries)->GetFunction());
}

NODE_MODULE(addon, init)
