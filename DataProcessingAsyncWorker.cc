#include <DataProcessingAsyncWorker.h>
// #include "uuid.h"
#include "BitBuffer.hpp"
#include <string.h>
#include "QrCode.hpp"
#include <iostream>
// #include "uuid.h"
#include <vector>
#include <random>
#include <sstream>
#include <string>
#include <chrono>
// #include <bits/stdc++.h>

// using namespace uuids;
using namespace std::chrono;
using qrcodegen::QrCode;
using qrcodegen::QrSegment;

DataProcessingAsyncWorker::DataProcessingAsyncWorker(int count,
                                                     Function &callback) : AsyncWorker(callback),
                                                                           //  nativeResponse(tempArray[count]),
                                                                           // responseData(std::string[count]),
                                                                           // responseData(Napi::Persistent(Napi::Object::New(Env()))),

                                                                           //    result(Object::New(Env())),
                                                                           pointerToResponse(new std::string[count]),
                                                                           pointerToResponseIds(new std::string[count]),
                                                                           count(count)
//    dataRef(ObjectReference::New(count, 1)),
//    dataPtr(data.Data()),
//    dataLength(data.Length())
{
    // pointerToResponse = new std::string [count];
}
unsigned int random_char()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    return dis(gen);
}

std::string intToHex(int number)
{
    std::stringstream sstream;
    sstream << std::hex << number;
    std::string result = sstream.str();
    // std::cout << number << ":"<< result << "\n";

    return result;
}
std::string mongoObjectId()
{
    milliseconds ms = duration_cast<milliseconds>(
        system_clock::now().time_since_epoch());
    std::string result = intToHex(ms.count() / 1000);
    // random generation
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(0, 15);
    for (int i = 0; i < 16; ++i)
    {
        // std::cout << intToHex(dist(mt)) << "\n";
        result.append(intToHex(dist(mt)));
    }

    return result;
}

void DataProcessingAsyncWorker::Execute()
{

    // std::cout << "DataProcessingAsyncWorker: started " << count << std::endl;

    for (int i = 0; i < count; i++)
    {
        std::string tempId = mongoObjectId();
        // to check dublicates
        //     if (std::find(name.begin(), name.end(), tempId) == name.end()) {
        //     // std::cout << "uuid " <<tempId  << std::endl;
        //   // someName not in name, add it
        //   name.push_back(tempId);
        // }else{
        //     std::cout << "dublicate uuid " <<tempId  << std::endl;

        // }

        // const char *text = "Hello, world! asdbasdjhagsdjha sdhgajhsdgajhsgdjhagsdjhagsjd asgdhjagsdjhagsdasd"; // User-supplied text
        const QrCode::Ecc errCorLvl = QrCode::Ecc::MEDIUM;
        const QrCode qr = QrCode::encodeText(tempId.c_str(), errCorLvl);
        // const QrCode qr = QrCode::encodeText(text, errCorLvl);

        pointerToResponse[i] = qr.toSvgString(1);
        pointerToResponseIds[i] = tempId;
        // std::cout << pointerToResponse[i]  << std::endl;

        // napi_create_string_utf8(this->Env(), qr.toSvgString(1).c_str(), NAPI_AUTO_LENGTH, &tempString);
        // napi_set_element(this->Env(), qrcodeArray, i, tempString);

        // napi_create_string_utf8(this->Env(), qr.toSvgString(1).c_str(), NAPI_AUTO_LENGTH, &tempString);

        // responseData.Set(i, qr.toSvgString(1));
        // responseData.Set(i, tempString);
        // Receiver().Set("test", Napi::Object::New(Env())
        // napi_set_element(this->Env(), responseData, i, tempString);
    }
    // napi_value tempString;

    //
    // for (size_t i = 0; i < dataLength; i++)
    // {
    //     uint8_t value = *(dataPtr + i);
    //     *(dataPtr + i) = value * 2;
    // }
}

void DataProcessingAsyncWorker::OnOK()
{
    // std::cout << "DataProcessingAsyncWorker: " << pointerToResponse[0] << std::endl;
    //! covert c++ array to js array, since js type cannot be used in execute and multitreading is only done for code written in execute
    napi_value tempString;
    napi_value tempStringId;
    napi_value qrcodeArray;

    napi_create_array_with_length(this->Env(), count, &qrcodeArray);

    for (int i = 0; i < count; i++)
    {
        Napi::Object obj = Napi::Object::New(this->Env());
        napi_create_string_utf8(this->Env(), pointerToResponse[i].c_str(), NAPI_AUTO_LENGTH, &tempString);
        napi_create_string_utf8(this->Env(), pointerToResponseIds[i].c_str(), NAPI_AUTO_LENGTH, &tempStringId);

        obj.Set(Napi::String::New(this->Env(), "svg"), tempString);
        obj.Set(Napi::String::New(this->Env(), "id"), tempStringId);
        napi_set_element(this->Env(), qrcodeArray, i, obj);
    }

    Callback().Call({
        qrcodeArray
        // Napi::Value(), //segfault
        //        responseData //promise with: Invalid pointer passed as argument
    });

    // dataRef.Unref();
}
