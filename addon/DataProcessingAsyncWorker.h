#include <napi.h>

using namespace Napi;

class DataProcessingAsyncWorker : public AsyncWorker
{
public:
    DataProcessingAsyncWorker(int count, std::string linkPrefix, std::string linkPostfix, int batchNumber, std::string batchId, Function &callback);

    void Execute();

    void OnOK();

private:
    // variables to convert c++ to js objects
    std::string *pointerToSvgs;
    std::string *pointerToUids;
    std::string *pointerToHashedUids;

    // parameters from js to c++
    int count;
    int batchNumber;
    std::string linkPrefix;
    std::string batchId;
    std::string linkPostfix;
};
