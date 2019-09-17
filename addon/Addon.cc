#include <napi.h>
#include <iostream>
#include <DataProcessingAsyncWorker.h>

using namespace Napi;

void ProcessData(const CallbackInfo &info)
{

    // parameters from javascript
    int count = info[0].ToNumber();
    std::string linkPrefix = info[1].ToString();
    std::string linkPostfix = info[2].ToString();
    int batchNumber = info[3].ToNumber();
    std::string batchId = info[4].ToString();
    Function cb = info[5].As<Function>();

    // std::cout << "process data started from c++, count: "<< count  << std::endl;
    DataProcessingAsyncWorker *worker = new DataProcessingAsyncWorker(count, linkPrefix, linkPostfix, batchNumber, batchId, cb);
    worker->Queue();
}

Object Init(Env env, Object exports)
{
    exports.Set(String::New(env, "processData"),
                Function::New(env, ProcessData));
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
