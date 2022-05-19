/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   3, NOV, 2021
 */

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "conv.h"

/* current charset */
u_char	current_system_string[8] = "charset";
int		current_system_charset = conv_charset::emUnknow;

 /* tag_conv_charset */
typedef struct ST_CONV_CHARSET tag_conv_charset;

/* ST_CONV_CHARSET */
struct ST_CONV_CHARSET
{
	int					type;
	std::string			utf8;
	std::string			ascii;
	std::wstring		unicode;
};

void conv_charset_utf8_to_ascii(tag_conv_charset * charset);
void conv_charset_utf8_to_unicode(tag_conv_charset * charset);
void conv_charset_unicode_to_utf8(tag_conv_charset * charset);
void conv_charset_unicode_to_ascii(tag_conv_charset * charset);
void conv_charset_ascii_to_utf8(tag_conv_charset * charset);
void conv_charset_ascii_to_unicode(tag_conv_charset * charset);

/* conv_charset_utf8_to_ascii */
void conv_charset_utf8_to_ascii(tag_conv_charset * charset)
{
	int			 len = 0;
	char		*str;
	wchar_t		*wstr;

	if (nullptr == setlocale(LC_ALL, "chs.utf8")) {
		return;
	}

	len = mbstowcs(nullptr, (char *)charset->utf8.data(), 0) + 1;
	if (len == 0) {
		return;
	}

	wstr = new wchar_t[len];
	mbstowcs(wstr, (char *)charset->utf8.data(), len);

	if (nullptr == setlocale(LC_ALL, "chs")) {
		return;
	}

	len = wcstombs(nullptr, wstr, 0) + 1;
	str = new char[len];
	wcstombs(str, wstr, len);

	charset->ascii = str;

	delete[](str);
	delete[](wstr);
}

/* conv_charset_utf8_to_unicode */
void conv_charset_utf8_to_unicode(tag_conv_charset * charset)
{
	int		 len = 0;
	wchar_t *wstr;

	if (nullptr == setlocale(LC_ALL, "chs.utf8")) {
		return;
	}

	len = mbstowcs(nullptr, (char *)charset->utf8.data(), 0) + 1;
	wstr = new wchar_t[len];
	mbstowcs(wstr, (char *)charset->utf8.data(), len);

	charset->unicode = wstr;
	delete[] wstr;
}

/* conv_charset_unicode_to_utf8 */
void conv_charset_unicode_to_utf8(tag_conv_charset * charset)
{
	int			 len = 0;
	char		*str;
	wchar_t		*wstr;

	if (nullptr == setlocale(LC_ALL, "zh_CN")) {
		return;
	}

	len = mbstowcs(nullptr, (char *)charset->ascii.data(), 0) + 1;
	wstr = new wchar_t[len];
	mbstowcs(wstr, (char *)charset->ascii.data(), len);

	if (nullptr == setlocale(LC_ALL, "chs")) {
		return;
	}

	len = wcstombs(nullptr, (wchar_t *)wstr, 0) + 1;
	str = new char[len];
	wcstombs(str, (wchar_t *)wstr, len);

	charset->utf8 = str;

	delete[](str);
	delete[](wstr);
}

/* conv_charset_unicode_to_ascii */
void conv_charset_unicode_to_ascii(tag_conv_charset * charset)
{
	int			 len = 0;
	wchar_t		*wstr;

	if (nullptr == setlocale(LC_ALL, "zh_CN")) {
		return;
	}

	len = mbstowcs(nullptr, (char *)charset->ascii.data(), 0) + 1;
	wstr = new wchar_t[len];
	mbstowcs(wstr, (char *)charset->ascii.data(), len);

	charset->unicode = wstr;
	delete[](wstr);
}

/* conv_charset_ascii_to_utf8 */
void conv_charset_ascii_to_utf8(tag_conv_charset * charset)
{
	int		 len = 0;
	char	*str;

	if (nullptr == setlocale(LC_ALL, "chs")) {
		return;
	}

	len = wcstombs(nullptr, (wchar_t *)charset->unicode.data(), 0) + 1;
	str = new char[len];
	wcstombs(str, (wchar_t *)charset->unicode.data(), len);

	charset->utf8 = str;
	delete[] str;
}

/* conv_charset_ascii_to_unicode */
void conv_charset_ascii_to_unicode(tag_conv_charset * charset)
{
	int			 len;
	char		*str;

	if (nullptr == setlocale(LC_ALL, "chs")) {
		return;
	}

	len = wcstombs(nullptr, (wchar_t *)charset->unicode.data(), 0) + 1;
	str = new char[len];
	wcstombs(str, (wchar_t *)charset->unicode.data(), len);

	charset->ascii = str;
	delete[] str;
}

/* conv_charset_utf8 */
u_char * conv_current_charset(std::string & result, u_char * data, int len, int type)
{
#if (!WINX)
	result.clear();
	result = (char *)data;
	return data;
#endif

	tag_conv_charset charset;

	if (type == conv_charset::emUtf8) {
		charset.utf8 = (char *)data;
	}
	else if (type == conv_charset::emAscii) {
		charset.ascii = (char *)data;
	}
	else if (type == conv_charset::emUnicode) {
		charset.unicode = (wchar_t*)data;
	}

#if (WINX)
	conv_charset_utf8_to_ascii(&charset);
#endif

	result.clear();
	result = charset.ascii;

	return nullptr;
}