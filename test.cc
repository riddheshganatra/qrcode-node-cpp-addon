#include <cstdlib>
#include <cstring>
#include <string>

#include <errno.h>

#include <nan.h>

#include <qrencode.h>
#include <png.h>

// for compatibility with libqrencode < 3.2.0:
#ifndef QRSPEC_VERSION_MAX
#define QRSPEC_VERSION_MAX 40
#endif

const unsigned int QRC_MAX_SIZE[] = { 2938, 2319, 1655, 1268 };
const int32_t COLOR_MAX = 0xFFFFFF;

struct Qrc_Params {
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
			int p_dot_size = 3, int p_margin = 4,
			int p_foreground_color = 0x0, int p_background_color = 0xffffff) {
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

	~Qrc_Params() {
		delete data;
	}
};

struct Qrc_Png_Buffer {
	char *data;
	uint32_t size;
	Qrc_Png_Buffer() {
		data = NULL;
		size = 0;
	}
	~Qrc_Png_Buffer() {
		free(data);
	}
};

Qrc_Params* ValidateArgs(const Nan::FunctionCallbackInfo<v8::Value>& info) {
	bool	success(false);
	struct Qrc_Params	*params = NULL;

	if (info.Length() < 1 || !info[0]->IsString()) {
		Nan::ThrowTypeError("No source string given");
		goto out;
	}
	params = new Qrc_Params(*Nan::Utf8String(info[0]));

	if (info.Length() > 1) {
		if (!info[1]->IsObject()) {
			Nan::ThrowTypeError("Second argument must be an object");
			goto out;
		}
		Nan::MaybeLocal<v8::Object> maybeParamsObj = Nan::To<v8::Object>(info[1]);
		if (maybeParamsObj.IsEmpty())
			goto out;
		v8::Local<v8::Object> paramsObj = maybeParamsObj.ToLocalChecked();

		Nan::MaybeLocal<v8::Value> maybeParamsVersion = Nan::Get(paramsObj, Nan::New("version").ToLocalChecked());
		if (maybeParamsVersion.IsEmpty())
			goto out;
		v8::Local<v8::Value> paramsVersion = maybeParamsVersion.ToLocalChecked();
		if (!paramsVersion->IsUndefined()) {
			if (!paramsVersion->IsInt32()) {
				Nan::ThrowTypeError("Wrong type for version");
				goto out;
			}
			Nan::MaybeLocal<v8::Int32> maybeVersion = Nan::To<v8::Int32>(paramsVersion);
			if (maybeVersion.IsEmpty())
				goto out;
			v8::Local<v8::Int32> version = maybeVersion.ToLocalChecked();
			if (version->Value() < 1 || version->Value() > QRSPEC_VERSION_MAX) {
				Nan::ThrowRangeError("Version out of range");
				goto out;
			}
			params->version = version->Value();
		}

		Nan::MaybeLocal<v8::Value> maybeParamsEcLevel = Nan::Get(paramsObj, Nan::New("ecLevel").ToLocalChecked());
		if (maybeParamsEcLevel.IsEmpty())
			goto out;
		v8::Local<v8::Value> paramsEcLevel = maybeParamsEcLevel.ToLocalChecked();
		if (!paramsEcLevel->IsUndefined()) {
			if (!paramsEcLevel->IsInt32()) {
				Nan::ThrowTypeError("Wrong type for EC level");
				goto out;
			}
			Nan::MaybeLocal<v8::Int32> maybeEcLevel = Nan::To<v8::Int32>(paramsEcLevel);
			if (maybeEcLevel.IsEmpty())
				goto out;
			v8::Local<v8::Int32> ecLevel = maybeEcLevel.ToLocalChecked();
			if (ecLevel->Value() < QR_ECLEVEL_L || ecLevel->Value() > QR_ECLEVEL_H) {
				Nan::ThrowRangeError("EC level out of range");
				goto out;
			}
			params->ec_level = (QRecLevel) ecLevel->Value();
		}

		Nan::MaybeLocal<v8::Value> maybeParamsMode = Nan::Get(paramsObj, Nan::New("mode").ToLocalChecked());
		if (maybeParamsMode.IsEmpty())
			goto out;
		v8::Local<v8::Value> paramsMode = maybeParamsMode.ToLocalChecked();
		if (!paramsMode->IsUndefined()) {
			if (!paramsMode->IsInt32()) {
				Nan::ThrowTypeError("Wrong type for mode");
				goto out;
			}
			Nan::MaybeLocal<v8::Int32> maybeMode = Nan::To<v8::Int32>(paramsMode);
			if (maybeMode.IsEmpty())
				goto out;
			v8::Local<v8::Int32> mode = maybeMode.ToLocalChecked();
			if (mode->Value() < QR_MODE_NUM || mode->Value() > QR_MODE_KANJI) {
				Nan::ThrowRangeError("Mode out of range");
				goto out;
			}
			params->mode = (QRencodeMode) mode->Value();
			// TODO check length of data
		}

		Nan::MaybeLocal<v8::Value> maybeParamsDotSize = Nan::Get(paramsObj, Nan::New("dotSize").ToLocalChecked());
		if (maybeParamsDotSize.IsEmpty())
			goto out;
		v8::Local<v8::Value> paramsDotSize = maybeParamsDotSize.ToLocalChecked();
		if (!paramsDotSize->IsUndefined()) {
			if (!paramsDotSize->IsInt32()) {
				Nan::ThrowTypeError("Wrong type for dot size");
				goto out;
			}
			Nan::MaybeLocal<v8::Int32> maybeDotSize = Nan::To<v8::Int32>(paramsDotSize);
			if (maybeDotSize.IsEmpty())
				goto out;
			v8::Local<v8::Int32> dotSize = maybeDotSize.ToLocalChecked();
			if (dotSize->Value() < 1 || dotSize->Value() > 50) {
				Nan::ThrowRangeError("Dot size out of range");
				goto out;
			}
			params->dot_size = dotSize->Value();
		}

		Nan::MaybeLocal<v8::Value> maybeParamsMargin = Nan::Get(paramsObj, Nan::New("margin").ToLocalChecked());
		if (maybeParamsMargin.IsEmpty())
			goto out;
		v8::Local<v8::Value> paramsMargin = maybeParamsMargin.ToLocalChecked();
		if (!paramsMargin->IsUndefined()) {
			if (!paramsMargin->IsInt32()) {
				Nan::ThrowTypeError("Wrong type for margin size");
				goto out;
			}
			Nan::MaybeLocal<v8::Int32> maybeMargin = Nan::To<v8::Int32>(paramsMargin);
			if (maybeMargin.IsEmpty())
				goto out;
			v8::Local<v8::Int32> margin = maybeMargin.ToLocalChecked();
			if (margin->Value() < 0 || margin->Value() > 10) {
				Nan::ThrowRangeError("Margin size out of range");
				goto out;
			}
			params->margin = margin->Value();
		}

		Nan::MaybeLocal<v8::Value> maybeParamsFgColor = Nan::Get(paramsObj, Nan::New("foregroundColor").ToLocalChecked());
		if (maybeParamsFgColor.IsEmpty())
			goto out;
		v8::Local<v8::Value> paramsFgColor = maybeParamsFgColor.ToLocalChecked();
		if (!paramsFgColor->IsUndefined()) {
			if (!paramsFgColor->IsInt32()) {
				Nan::ThrowTypeError("Wrong type for foreground color");
				goto out;
			}
			Nan::MaybeLocal<v8::Int32> maybeFgColor = Nan::To<v8::Int32>(paramsFgColor);
			if (maybeFgColor.IsEmpty())
				goto out;
			v8::Local<v8::Int32> fgColor = maybeFgColor.ToLocalChecked();
			if (fgColor->Value() < 0 || fgColor->Value() > COLOR_MAX) {
				Nan::ThrowRangeError("Foreground color out of range");
				goto out;
			}
			params->foreground_color = fgColor->Value();
		}

		Nan::MaybeLocal<v8::Value> maybeParamsBgColor = Nan::Get(paramsObj, Nan::New("backgroundColor").ToLocalChecked());
		if (maybeParamsBgColor.IsEmpty())
			goto out;
		v8::Local<v8::Value> paramsBgColor = maybeParamsBgColor.ToLocalChecked();
		if (!paramsBgColor->IsUndefined()) {
			if (!paramsBgColor->IsInt32()) {
				Nan::ThrowTypeError("Wrong type for background color");
				goto out;
			}
			Nan::MaybeLocal<v8::Int32> maybeBgColor = Nan::To<v8::Int32>(paramsBgColor);
			if (maybeBgColor.IsEmpty())
				goto out;
			v8::Local<v8::Int32> bgColor = maybeBgColor.ToLocalChecked();
			if (bgColor->Value() < 0 || bgColor->Value() > COLOR_MAX) {
				Nan::ThrowRangeError("Background color out of range");
				goto out;
			}
			params->background_color = bgColor->Value();
		}
	}

	if (params->datalen < 1 || params->datalen > QRC_MAX_SIZE[params->ec_level]) {
		Nan::ThrowRangeError("Source string length out of range");
		goto out;
	}

	success = true;
	/* FALLTHROUGH */
out:
	if (!success) {
		delete params;
		params = NULL;
	}
	return (params);
}


QRcode *Encode(Qrc_Params *params) {
	// reset errno, so the caller is able to distinguish distinct error cases.
	errno = 0;
	QRcode	*code  = NULL;
	QRinput	*input = NULL;

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


void EncodeBuf(const Nan::FunctionCallbackInfo<v8::Value>& info) {
	v8::Local<v8::Object> obj = Nan::New<v8::Object>();
	QRcode *code = NULL;

	Qrc_Params *params = ValidateArgs(info);
	if (!params)
		goto out;

	code = Encode(params);
	delete params;
	if (code == NULL) {
		if (errno == EINVAL) {
			Nan::ThrowError("Input data is invalid");
		} else if (errno == ENOMEM) {
			Nan::ThrowError("Not enough memory");
		} else {
			Nan::ThrowError("Could not encode input");
		}
	} else {
		// XXX: overflow check on (code->width * code->width) ?
		Nan::MaybeLocal<v8::Object> maybeBuffer = Nan::CopyBuffer((const char *)code->data, code->width * code->width);
		if (!maybeBuffer.IsEmpty()) {
			Nan::Set(obj, Nan::New("data").ToLocalChecked(), maybeBuffer.ToLocalChecked());
			Nan::Set(obj, Nan::New("width").ToLocalChecked(),   Nan::New(code->width));
			Nan::Set(obj, Nan::New("version").ToLocalChecked(), Nan::New(code->version));
		}
	}
	/* FALLTHROUGH */
out:
	QRcode_free(code);
	info.GetReturnValue().Set(obj);
}


void Qrc_png_write_buffer(png_structp png_ptr, png_bytep data, png_size_t length) {
	Qrc_Png_Buffer *b = (Qrc_Png_Buffer *)png_get_io_ptr(png_ptr);
	size_t nsize = b->size + length; // FIXME: overflow check anyone?

	char *old = b->data;
	b->data = (char *)realloc(b->data, nsize);

	if (!b->data) {
		free(old);
		png_error(png_ptr, "Write Error");
	}

	memcpy(b->data + b->size, data, length);
	b->size += length;
}


void EncodePNG(const Nan::FunctionCallbackInfo<v8::Value>& info) {
	v8::Local<v8::Object> obj = Nan::New<v8::Object>();

	Qrc_Params* params = ValidateArgs(info);
	if (!params) {
		info.GetReturnValue().Set(obj);
		return;
	}

	QRcode *code = Encode(params);
	if (code == NULL) {
		if (errno == EINVAL) {
			Nan::ThrowError("Input data is invalid");
		} else if (errno == ENOMEM) {
			Nan::ThrowError("Not enough memory");
		} else {
			Nan::ThrowError("Could not encode input");
		}
	} else {
		Qrc_Png_Buffer* bp = new Qrc_Png_Buffer();

		png_structp png_ptr;
		png_infop info_ptr;
		png_colorp png_plte;

		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
				NULL, NULL, NULL);
		if (!png_ptr) {
			delete params;
			QRcode_free(code);
			info.GetReturnValue().Set(obj);
			return;
		}

		info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr) {
			png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
			delete params;
			QRcode_free(code);
			info.GetReturnValue().Set(obj);
			return;
		}

		if (setjmp(png_jmpbuf(png_ptr))) {
			png_destroy_write_struct(&png_ptr, &info_ptr);
			delete params;
			QRcode_free(code);
			info.GetReturnValue().Set(obj);
			return;
		}

		png_set_write_fn(png_ptr, bp, Qrc_png_write_buffer, NULL);

		png_plte = (png_colorp) malloc(sizeof(png_color) * 2);
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

		unsigned char* row = new unsigned char[(code->width + params->margin * 2) * params->dot_size];

		for (int y = -(params->margin); y < code->width + params->margin; y++) {
			for (int x = -(params->margin * params->dot_size); x < (code->width + params->margin) * params->dot_size; x += params->dot_size) {
				for (int d = 0; d < params->dot_size; d++) {
					if (y < 0 || y > code->width - 1 || x < 0 || x > ((code->width - 1) * params->dot_size)) {
						row[x + params->margin * params->dot_size + d] = 0;
					} else {
						row[x + params->margin * params->dot_size + d] = code->data[y * code->width + x/params->dot_size] << 7;
					}
				}
			}
			for (int d = 0; d < params->dot_size; d++) {
				png_write_row(png_ptr, row);
			}
		}

		png_write_end(png_ptr, info_ptr);
		png_destroy_write_struct(&png_ptr, &info_ptr);

		delete[] row;
		free(png_plte);

		Nan::MaybeLocal<v8::Object> maybeBuffer = Nan::CopyBuffer(bp->data, bp->size);
		if (!maybeBuffer.IsEmpty()) {
			Nan::Set(obj, Nan::New("data").ToLocalChecked(), maybeBuffer.ToLocalChecked());
			Nan::Set(obj, Nan::New("width").ToLocalChecked(),   Nan::New(code->width));
			Nan::Set(obj, Nan::New("version").ToLocalChecked(), Nan::New(code->version));
		}
		QRcode_free(code);
		delete bp;
	}
	delete params;
	info.GetReturnValue().Set(obj);
}

void Init(v8::Local<v8::Object> exports) {
	exports->Set(Nan::New("encode").ToLocalChecked(),
	    Nan::New<v8::FunctionTemplate>(EncodeBuf)->GetFunction());
	exports->Set(Nan::New("encodePng").ToLocalChecked(),
	    Nan::New<v8::FunctionTemplate>(EncodePNG)->GetFunction());
}

NODE_MODULE(qrc, Init)