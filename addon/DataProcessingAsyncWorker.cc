#include <DataProcessingAsyncWorker.h>
#include <fstream>
// #include <bits/stdc++.h>
// #include "uuid.h"
#include <string.h>
#include <iostream>
// #include "uuid.h"
#include <vector>
#include <random>
#include <sstream>
#include <string>
#include <chrono>
#include "picosha2.h"
#include <qrencode.h>
#include <png.h>
#include <unistd.h>
#include "base64.h"
#include <thread>

using namespace std::chrono;
using std::fstream;

#ifndef QRSPEC_VERSION_MAX
#define QRSPEC_VERSION_MAX 40
#endif

// start of qrcode helper function
//* code copied from existing package https://github.com/netoxygen/node-qrcodeine

// const unsigned int QRC_MAX_SIZE[] = {2938, 2319, 1655, 1268};
// const int32_t COLOR_MAX = 0xFFFFFF;
// compression https://www.codeproject.com/Articles/8212/Zip-bytes-in-memory-and-unzip-file-into-bytes-buff

struct Qrc_Params
{
	unsigned char *data;
	size_t datalen;
	QRecLevel ec_level;
	QRencodeMode mode;
	int dot_size;
	int margin;
	int foreground_color;
	int background_color;
	int version;

	Qrc_Params(const std::string &p_data, QRecLevel p_ec_level = QR_ECLEVEL_L, QRencodeMode p_mode = QR_MODE_8,
			   int p_version = 0,
			   int p_dot_size = 10, int p_margin = 4,
			   int p_foreground_color = 0x0, int p_background_color = 0xffffff)
	{
		datalen = p_data.length();
		data = new unsigned char[datalen + 1];
		std::strncpy((char *)data, p_data.c_str(), datalen + 1);
		ec_level = p_ec_level;
		mode = p_mode;
		version = p_version;
		dot_size = p_dot_size;
		margin = p_margin;
		foreground_color = p_foreground_color;
		background_color = p_background_color;
	}

	~Qrc_Params()
	{
		delete data;
	}
};

struct Qrc_Png_Buffer
{
	char *data;
	uint32_t size;
	Qrc_Png_Buffer()
	{
		data = NULL;
		size = 0;
	}
	~Qrc_Png_Buffer()
	{
		free(data);
	}
};

Qrc_Params *ValidateArgs(std::string textdata)
{
	struct Qrc_Params *params = NULL;

	params = new Qrc_Params(textdata);

	// if (params->datalen < 1 || params->datalen > QRC_MAX_SIZE[params->ec_level]) {
	// 	Nan::ThrowRangeError("Source string length out of range");
	// 	goto out;
	// }

	// success = true;
	/* FALLTHROUGH */
	return (params);
}

void Qrc_png_write_buffer(png_structp png_ptr, png_bytep data, png_size_t length)
{
	Qrc_Png_Buffer *b = (Qrc_Png_Buffer *)png_get_io_ptr(png_ptr);
	size_t nsize = b->size + length; // FIXME: overflow check anyone?

	char *old = b->data;
	b->data = (char *)realloc(b->data, nsize);

	if (!b->data)
	{
		free(old);
		png_error(png_ptr, "Write Error");
	}

	memcpy(b->data + b->size, data, length);
	b->size += length;
}

QRcode *Encode(Qrc_Params *params)
{
	// reset errno, so the caller is able to distinguish distinct error cases.
	errno = 0;
	QRcode *code = NULL;
	QRinput *input = NULL;

	input = QRinput_new2(params->version, params->ec_level);
	if (input == NULL)
		goto out;
	if (QRinput_append(input, params->mode, params->datalen, params->data) == -1)
		goto out;
	code = QRcode_encodeInput(input);

	/* FALLTHROUGH */
out:
	QRinput_free(input);
	return (code);
}
// end of qrcode helper function

// generate random mongo id using below reference, also have added logic for pid
// * reference from: https://gist.github.com/solenoid/1372386
std::string intToHex(int number)
{
	std::stringstream sstream;
	sstream << std::hex << number;
	std::string result = sstream.str();
	// std::cout << number << ":"<< result << "\n";

	return result;
}
std::string mongoObjectId(int batchNumber)
{
	milliseconds ms = duration_cast<milliseconds>(
		system_clock::now().time_since_epoch());
	std::string result = intToHex(ms.count() / 1000);
	// random generation
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> dist(0, 15);

	std::string hexBatchNo = intToHex(batchNumber);

	int batchLen = hexBatchNo.length();
	result.append(hexBatchNo);

	for (int i = 0; i < 16 - batchLen; ++i)
	{
		// std::cout << intToHex(dist(mt)) << "\n";
		result.append(intToHex(dist(mt)));
	}
	return result;
}

// *code reference from : https://codemerx.com/blog/asynchronous-c-addon-for-node-js-with-n-api-and-node-addon-api/
DataProcessingAsyncWorker::DataProcessingAsyncWorker(int count, std::string linkPrefix, std::string linkPostfix, int batchNumber, std::string batchId,
													 Function &callback) : AsyncWorker(callback),
																		   pointerToSvgs(new std::string[count]),
																		   pointerToUids(new std::string[count]),
																		   pointerToHashedUids(new std::string[count]),
																		   count(count),
																		   batchNumber(batchNumber),
																		   linkPrefix(linkPrefix),
																		   batchId(batchId),
																		   linkPostfix(linkPostfix)
{
	// pointerToResponse = new std::string [count];
}

void DataProcessingAsyncWorker::Execute()
{
	// fstream file;
	// file.open("foo.zip", fstream::out);
	// file.close();

	// std::cout << std::thread::hardware_concurrency() << std::endl;

	for (int i = 0; i < count; i++)
	{

		// generate new uuid
		pointerToUids[i] = mongoObjectId(batchNumber);

		pointerToHashedUids[i] = picosha2::hash256_hex_string(pointerToUids[i]);
		// std::cout << "DataProcessingAsyncWorker: started " << count << std::endl;

		// generate qrcode logic
		Qrc_Params *params = ValidateArgs(linkPrefix + pointerToHashedUids[i] + linkPostfix);

		QRcode *code = Encode(params);
		Qrc_Png_Buffer *bp = new Qrc_Png_Buffer();

		png_structp png_ptr;
		png_infop info_ptr;
		png_colorp png_plte;

		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
										  NULL, NULL, NULL);

		info_ptr = png_create_info_struct(png_ptr);

		png_set_write_fn(png_ptr, bp, Qrc_png_write_buffer, NULL);

		png_plte = (png_colorp)malloc(sizeof(png_color) * 2);
		png_plte[0].red = params->background_color >> 16 & 0xFF;
		png_plte[0].green = params->background_color >> 8 & 0xFF;
		png_plte[0].blue = params->background_color & 0xFF;
		png_plte[1].red = params->foreground_color >> 16 & 0xFF;
		png_plte[1].green = params->foreground_color >> 8 & 0xFF;
		png_plte[1].blue = params->foreground_color & 0xFF;

		png_set_PLTE(png_ptr, info_ptr, png_plte, 2);

		png_set_IHDR(png_ptr, info_ptr, (code->width + params->margin * 2) * params->dot_size, (code->width + params->margin * 2) * params->dot_size, 1,
					 PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
					 PNG_FILTER_TYPE_DEFAULT);

		png_write_info(png_ptr, info_ptr);

		png_set_packing(png_ptr);

		unsigned char *row = new unsigned char[(code->width + params->margin * 2) * params->dot_size];

		for (int y = -(params->margin); y < code->width + params->margin; y++)
		{
			for (int x = -(params->margin * params->dot_size); x < (code->width + params->margin) * params->dot_size; x += params->dot_size)
			{
				for (int d = 0; d < params->dot_size; d++)
				{
					if (y < 0 || y > code->width - 1 || x < 0 || x > ((code->width - 1) * params->dot_size))
					{
						row[x + params->margin * params->dot_size + d] = 0;
					}
					else
					{
						row[x + params->margin * params->dot_size + d] = code->data[y * code->width + x / params->dot_size] << 7;
					}
				}
			}
			for (int d = 0; d < params->dot_size; d++)
			{
				png_write_row(png_ptr, row);
			}
		}

		png_write_end(png_ptr, info_ptr);
		png_destroy_write_struct(&png_ptr, &info_ptr);

		delete[] row;
		free(png_plte);

		// std::cout << bp->data << "\n";
		// std::cout << ret << "\n";
		// std::cout << base64_encode(bp->data, bp->size) << "\n";
		// std::cout << base64_encode(reinterpret_cast<const unsigned char*>(ret.c_str()), ret.length()) << "\n";
		// reinterpret_cast<const unsigned char*>(s.c_str()), s.length())
		// std::cout << "after bp" << "\n";

		delete params;

		std::string ret(bp->data, bp->size);
		// delete[] bp;
		// push qrcode and id to array so they can be converted to js object in onOk()
		pointerToSvgs[i] = base64_encode(reinterpret_cast<const unsigned char *>(ret.c_str()), ret.length());

		if (batchId != "")
		{

			std::cout << ("./" + batchId + "/" + pointerToUids[i] + ".png").c_str() << "\n";
			std::ofstream outfile(("./" + batchId + "/" + pointerToUids[i] + ".png").c_str(), std::ofstream::binary);
			outfile.write(bp->data, bp->size);
			outfile.close();
		}

		// std::ofstream myfile;
		// myfile.open((pointerToUids[i] + ".png").c_str());
		// myfile << bp->data;
		// myfile.close();

		// {

		// }

		delete bp;
		QRcode_free(code);
		// delete ret;

		// const uint8_t PIXELS[] = {
		//     0xFF,
		//     0x00,
		//     0x00,
		//     0x00,
		//     0xFF,
		//     0x00,
		//     0x00,
		//     0x00,
		//     0xFF,
		//     0xFF,
		//     0x00,
		//     0xFF,
		//     0xFF,
		//     0xFF,
		//     0x00,
		//     0x00,
		//     0xFF,
		//     0xFF,
		// };
	}
	// if (batchNumber == 1)
	// {

	// }
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

	//  free memory for class
	delete[] pointerToSvgs;
	delete[] pointerToUids;
	delete[] pointerToHashedUids;
	// std::cout << "DataProcessingAsyncWorker: DONE" << std::endl;

	Callback().Call({
		qrcodeArray
		// Napi::Value(), //segfault
		//        responseData //promise with: Invalid pointer passed as argument
	});

	// dataRef.Unref();
}
