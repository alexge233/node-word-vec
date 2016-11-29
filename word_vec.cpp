#include <iostream>
#include <sstream>

#include <node.h>
#include <v8.h>

#include "cpp/tokenizer/tokenizer.hpp"
#include "cpp/miner/miner.hpp"
#include "cpp/principals/principals.hpp"
#include "cpp/semantics/semantics.hpp"
#include "cpp/compressor/compressor.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

using namespace v8;

data unpack_data(Isolate * isolate, const Handle<Object> row)
{
    data d;
    Handle<Value> text_value = row->Get(String::NewFromUtf8(isolate, "data"));
    Handle<Value> score_value = row->Get(String::NewFromUtf8(isolate, "score"));
    v8::String::Utf8Value utfValue(text_value);
    d.review = std::string(*utfValue);
    d.score = score_value->NumberValue();
    return std::move(d);
}

// get the JSON data and parse it into our dataset
std::vector<data> unpack_json(Isolate * isolate, const v8::FunctionCallbackInfo<v8::Value> &args)
{
    std::vector<data> json_data;
    Handle<Object> dataset = Handle<Object>::Cast(args[0]);
    Handle<Array> array = Handle<Array>::Cast(dataset->Get(String::NewFromUtf8(isolate, "dataset")));
    int data_len = array->Length();
    for (int i = 0; i < data_len; i++)
    {
        data row = unpack_data(isolate, Handle<Object>::Cast(array->Get(i)));
        json_data.push_back(row);
    }
    return json_data;
}

// pack the dataset and allocate it on the v8 heap
Local<Array> pack(Isolate * isolate, std::vector<data> & dataset)
{
    // array of data
    Local<Array> result = Array::New(isolate);
    // populate
    for (unsigned int i = 0; i < dataset.size(); i++)
    {
        Local<Object> obj = Object::New(isolate);
        // set object text
        obj->Set(String::NewFromUtf8(isolate, "text"),
                 String::NewFromUtf8(isolate, dataset[i].review.c_str()));
        // set object score
        obj->Set(String::NewFromUtf8(isolate, "score"),
                 Number::New(isolate, dataset[i].score));
        // allocate an array, set size
        Local<Array> vec = Array::New(isolate);
        // populate the v8 array (copy)
        for (unsigned int k = 0; k < dataset[i].vector.size(); k++)
        {
            vec->Set(k, Number::New(isolate, dataset[i].vector[k]));
        }
        obj->Set(String::NewFromUtf8(isolate, "vector"), vec);
        result->Set(i, obj);
    }
    return std::move(result);
}

//  argv[0]: the parsed json data
//  argv[1]: filter the reviews above `filter` length
//  argv[2]: encodable_threshold (int)
//  argv[3]: non_encodable_thres (int)
//  RETURN: ??
void compress_sparse(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    if (args.Length() > 0 && args.Length() < 6)
    {
        Isolate* isolate = args.GetIsolate();
        // unpack node data
        std::vector<data> dataset = unpack_json(isolate, args);
        // get params
        unsigned int f = args[1]->Uint32Value();
        unsigned int x = args[2]->Uint32Value();
        unsigned int y = args[3]->Uint32Value();
        // load tokenizer
        tokenizer tkr;
        // POS tag
        tkr(dataset);
        // filter
        dataset = tkr.filter(dataset, f); 
        unsigned int max_size = tkr.max_size(dataset);
        // semantics
        semantics sema_blob = semantics(dataset);
        std::vector<triplet> known_stats = miner()(dataset, sema_blob.known_words);
        std::unordered_set<word> enc_principals = principals()(known_stats, x);
        std::vector<triplet> unknown_stats = miner()(dataset, sema_blob.unknown_words);
        std::unordered_set<word> sym_principals = principals()(unknown_stats, y);
        // compress
        compressor algo(sema_blob, enc_principals, sym_principals);

        // compressed sparse (best delta) vector - WARNING: take care with last param!!!
        // setting to `false` will return a dense vector
        algo.compressed_data(dataset, max_size, true);
        // pack and allocate
        Local<Array> result = pack(isolate, dataset);
        // return it
        args.GetReturnValue().Set(result);
    }
    else
        throw std::runtime_error("illegal params");
}

//  TODO: return a compressed dense vector (all delta)
void compress_dense(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    // TODO: populate a dense vector
    /*
        tokenizer tkr;
        tkr(dataset);
        dataset = tkr.filter(dataset, filter); 
        unsigned int max_size = tkr.max_size(dataset);
        semantics sema_blob = semantics(dataset);
        std::vector<triplet> known_stats = miner()(dataset, sema_blob.known_words);
        std::unordered_set<word> enc_principals = principals()(known_stats, x);
        std::vector<triplet> unknown_stats = miner()(dataset, sema_blob.unknown_words);
        std::unordered_set<word> sym_principals = principals()(unknown_stats, y);
        compressor algo(sema_blob, enc_principals, sym_principals);
        algo.compressed_data(dataset, max_size, true);
        //algo.uncompressed_data(dataset, max_size);
    */
}

void init(Handle <Object> exports, Handle<Object> module)
{
    NODE_SET_METHOD(exports, "compress_sparse", compress_sparse);
    NODE_SET_METHOD(exports, "compress_dense", compress_dense);
}

NODE_MODULE(word_vec, init)
