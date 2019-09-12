#include <napi.h>

using namespace Napi;

class DataProcessingAsyncWorker : public AsyncWorker
{
    public:
        DataProcessingAsyncWorker(int count,std::string linkPrefix,
                                  Function &callback);

        void Execute();

        void OnOK();

    private:
        // ObjectReference dataRef;
        // uint8_t *dataPtr;
        // size_t dataLength;
        // Napi::ObjectReference responseData;
        // std::string responseData[];
        std::string *pointerToSvgs;
        std::string *pointerToUids;
        std::string *pointerToHashedUids;
        int count;
        std::string linkPrefix;
};
