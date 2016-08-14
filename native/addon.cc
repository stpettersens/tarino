/*
  tarino: a tar implementation with Node.js.
  Copyright 2016 Sam Saint-Pettersen.

  Native add-on.

  Dual licensed under the GPL and MIT licenses;
  see GPL-LICENSE and MIT-LICENSE respectively.
*/

#include <nan.h>
#include <string>
#include "tarino.hpp"

void export_write_tar_entry(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    if (info.Length() < 5) {
        Nan::ThrowTypeError("tarino-native (write_tar_entry): Wrong number of arguments");
        return;
    }

    if (!info[0]->IsString() || !info[1]->IsString()
    || !info[2]->IsNumber() || !info[3]->IsNumber() || !info[4]->IsNumber()) {
        Nan::ThrowTypeError("tarino-native (write_tar_entry): Arguments should be string, string, number (int), number (int)");
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
        Nan::ThrowTypeError("tarino-native (write_tar_entries): Wrong number of arguments");
        return;
    }

    if (!info[0]->IsString() || !info[1]->IsString()) {
        Nan::ThrowTypeError("tarino-native (write_tar_entries): Arguments should be string, string");
        return;
    }

    v8::String::Utf8Value tarname(info[0]->ToString());
    v8::String::Utf8Value manifest(info[1]->ToString());
    v8::Local<v8::Number> code = Nan::New(
    write_tar_entries(std::string(*tarname), std::string(*manifest)));

    info.GetReturnValue().Set(code);
}

void export_extract_tar_entries(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    if (info.Length() < 4) {
        Nan::ThrowTypeError("tarino-native (extract_tar_entries): Wrong number of arguments");
        return;
    }

    if (!info[0]->IsString() || !info[1]->IsNumber() || !info[2]->IsNumber() || !info[3]->IsNumber()) {
        Nan::ThrowTypeError("tarino-native (extract_tar_entries): Arguments should be string, number, number (0 or 1) x2");
        return;
    }

    v8::String::Utf8Value tarname(info[0]->ToString());
    int size = (int)info[1]->NumberValue();
    int overwrite = (int)info[2]->NumberValue();
    int verbose = (int)info[3]->NumberValue();
    v8::Local<v8::Number> code = Nan::New(
    extract_tar_entries(std::string(*tarname), size, overwrite, verbose));

    info.GetReturnValue().Set(code);
}

void export_list_tar_entries(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    if (info.Length() < 3) {
        Nan::ThrowTypeError("tarino-native (list_tar_entries): Wrong number of arguments");
        return;
    }

    if (!info[0]->IsString() || !info[1]->IsNumber() || !info[2]->IsNumber()) {
        Nan::ThrowTypeError("tarino-native (list_tar_entries): Arguments should be string, number, number (0 or 1)");
        return;
    }

    v8::String::Utf8Value tarname(info[0]->ToString());
    int size = (int)info[1]->NumberValue();
    int verbose = (int)info[2]->NumberValue();
    v8::Local<v8::Number> code = Nan::New(
    list_tar_entries(std::string(*tarname), size, verbose));

    info.GetReturnValue().Set(code);
}

void init(v8::Local<v8::Object> exports) {
    exports->Set(Nan::New("write_tar_entry").ToLocalChecked(),
    Nan::New<v8::FunctionTemplate>(export_write_tar_entry)->GetFunction());

    exports->Set(Nan::New("write_tar_entries").ToLocalChecked(),
    Nan::New<v8::FunctionTemplate>(export_write_tar_entries)->GetFunction());

    exports->Set(Nan::New("extract_tar_entries").ToLocalChecked(),
    Nan::New<v8::FunctionTemplate>(export_extract_tar_entries)->GetFunction());

    exports->Set(Nan::New("list_tar_entries").ToLocalChecked(),
    Nan::New<v8::FunctionTemplate>(export_list_tar_entries)->GetFunction());
}

NODE_MODULE(addon, init)
