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
#include "picosha2.h"
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
                                                                           pointerToSvgs(new std::string[count]),
                                                                           pointerToUids(new std::string[count]),
                                                                           pointerToHashedUids(new std::string[count]),
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
        // generate new uuid
        pointerToUids[i] = mongoObjectId();
         
        pointerToHashedUids[i]=picosha2::hash256_hex_string(pointerToUids[i]);
            // std::cout << "hash " <<picosha2::hash256_hex_string(tempId)  << std::endl;

            // std::cout << "hash " <<std::hash<std::string>{}(tempId)  << std::endl;
        // to check dublicates
        //     if (std::find(name.begin(), name.end(), tempId) == name.end()) {
        //     // std::cout << "uuid " <<tempId  << std::endl;
        //   // someName not in name, add it
        //   name.push_back(tempId);
        // }else{
        //     std::cout << "dublicate uuid " <<tempId  << std::endl;

        // }

        // new qrcode
        const QrCode::Ecc errCorLvl = QrCode::Ecc::LOW;
        const QrCode qr = QrCode::encodeText(pointerToHashedUids[i].c_str(), errCorLvl);

        // push qrcode and id to array so they can be converted to js object in onOk()
        pointerToSvgs[i] = qr.toSvgString(0);
    }
}

void DataProcessingAsyncWorker::OnOK()
{
    // std::cout << "DataProcessingAsyncWorker: " << pointerToResponse[0] << std::endl;
    //! covert c++ array to js array, since js type cannot be used in execute and multitreading is only done for code written in execute
    napi_value svgString;
    napi_value uid;
    napi_value hashedUid;
    napi_value qrcodeArray;

    napi_create_array_with_length(this->Env(), count, &qrcodeArray);

    for (int i = 0; i < count; i++)
    {
        Napi::Object tempObj = Napi::Object::New(this->Env());
        napi_create_string_utf8(this->Env(), pointerToSvgs[i].c_str(), NAPI_AUTO_LENGTH, &svgString);
        napi_create_string_utf8(this->Env(), pointerToUids[i].c_str(), NAPI_AUTO_LENGTH, &uid);
        napi_create_string_utf8(this->Env(), pointerToHashedUids[i].c_str(), NAPI_AUTO_LENGTH, &hashedUid);

        tempObj.Set(Napi::String::New(this->Env(), "svg"), svgString);
        tempObj.Set(Napi::String::New(this->Env(), "id"), uid);
        tempObj.Set(Napi::String::New(this->Env(), "hash"), hashedUid);
        napi_set_element(this->Env(), qrcodeArray, i, tempObj);
    }

    Callback().Call({
        qrcodeArray
        // Napi::Value(), //segfault
        //        responseData //promise with: Invalid pointer passed as argument
    });

    // dataRef.Unref();
}
