///////////////////////////////////////////////////////////////////////////////
// vim:         ts=4 sw=4
// Name:        wxmedit/wxm_encoding_multibyte.cpp
// Description: define the Multi-byte Encodings which are supported by wxMEdit
// Author:      wxmedit@gmail.com
// Licence:     GPL
///////////////////////////////////////////////////////////////////////////////

#include "wxm_encoding_multibyte.h"
#include <boost/scoped_array.hpp>

#ifdef _DEBUG
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK ,__FILE__, __LINE__)
#endif

namespace wxm
{

extern "C"
{
	void error_callback(const void *context, UConverterFromUnicodeArgs *args, const UChar *codeUnits, int32_t length, UChar32 codePoint, UConverterCallbackReason reason, UErrorCode *pErrorCode)
	{
		*pErrorCode = U_INVALID_CHAR_FOUND;
	}
}

ICUConverter::ICUConverter(const std::string& encname): m_ucnv(NULL)
{
	UErrorCode err = U_ZERO_ERROR;

	m_ucnv = ucnv_open(encname.c_str(), &err);

	ucnv_setFallback(m_ucnv, FALSE);
	ucnv_setFromUCallBack(m_ucnv, error_callback, NULL, NULL, NULL, &err);
}

ICUConverter::~ICUConverter()
{
	ucnv_close(m_ucnv);
	m_ucnv = NULL;
}

size_t ICUConverter::MB2WC(UChar32& ch, const char* src, size_t src_len)
{
	UChar dest[2];
	UErrorCode err = U_ZERO_ERROR;
	int32_t n = ucnv_toUChars(m_ucnv, dest, 2, src, src_len, &err);

	if (n == 1)
	{
		if (dest[0]==0xFFFD)
			return 0;

		ch = dest[0];
		return 1;
	}

	if (n<1 || n>=2 && (dest[0]<0xD800 || dest[0]>0xDBFF || dest[1]<0xDC00 || dest[1]>0xDFFF))
		return 0;

	ch = 0x10000 + (((dest[0] & 0x03FF) << 10) | (dest[1] & 0x03FF));

	return 1;
}

size_t ICUConverter::WC2MB(char* dest, size_t dest_len, const UChar32& ch)
{
	UChar src[2] = {0, 0};
	size_t src_len = 1;
	if (ch <= 0xFFFF)
	{
		src[0] = ch;
	}
	else
	{
		UChar32 tmp = ch - 0x10000;
		src[0] = (ch >> 10) | 0xD800;
		src[1] = (ch & 0x3FF) | 0xDC00;
	}

	UErrorCode err = U_ZERO_ERROR;
	int32_t n = ucnv_fromUChars(m_ucnv, dest, dest_len, src, src_len, &err);

	if (n<=0)
		return 0;

	return ((size_t)n>dest_len)? dest_len: (size_t)n;
}

void WXMEncodingMultiByte::Create(ssize_t idx)
{
	WXMEncoding::Create(idx);

	MultiByteInit();
}

};// namespace wxm
