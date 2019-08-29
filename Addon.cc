#include <napi.h>
#include <iostream>
#include <DataProcessingAsyncWorker.h>

using namespace Napi;

void ProcessData(const CallbackInfo& info) {
    // Buffer<uint8_t> data = info[0].As<Buffer<uint8_t>>();

    int count = info[0].ToNumber();
    // Napi::Object obj = info[1].As<Napi::Object>();
    // Napi::Array props = obj.GetPropertyNames();

    // // std::cout << "object props " <<props  << std::endl;

    // std::cout << "object props " <<props.Length()  << std::endl;
    // for (unsigned int j = 0; j < props.Length(); j++) {
    // std::cout << "object props " << props.Get(j)  << std::endl;


                // printf("%s: %s",
                //        *v8::String::Utf8Value(props->Get(j)->ToString()),
                //        *v8::String::Utf8Value(obj->Get(props->Get(j))->ToString())
                //       );
            // }


    Function cb = info[1].As<Function>();

        // std::cout << "process data started from c++, count: "<< count  << std::endl;
    DataProcessingAsyncWorker *worker = new DataProcessingAsyncWorker(count, cb);
    worker->Queue();
}

Object Init(Env env, Object exports) {
    exports.Set(String::New(env, "processData"),
                Function::New(env, ProcessData));
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
