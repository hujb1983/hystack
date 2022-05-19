/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, FEB, 2021
 */

#pragma once
#include <string>
#include <core_config.h>

#if (WINX)
#include <windows.h>
#endif

/* crypto_base64_encode */
int conv_base64_encode(unsigned char * dst, unsigned int len, const std::string & src);

/* crypto_base64_encode */
int conv_base64_encode(unsigned char * bind, unsigned char *b64, unsigned int bin_len);

/* conv_base64_decode */
int conv_base64_decode(unsigned char * dst, unsigned int len, const std::string & src);

/* conv_md5_encode */
void conv_md5_encode(char * result, unsigned char type, const std::string & text);

/* conv_rsa_decode */
void conv_rsa_encode(int e, int d, int n, int value, FILE* out);

/* conv_rsa_decode */
void conv_rsa_decode(int e, int d, int n, int value, FILE* out);


/* conv_charset */
struct conv_charset
{
	enum {
		emUtf8,
		emAscii,
		emUnicode,
		emUnknow
	};
};

/* conv_charset_utf8 */
u_char * conv_current_charset(std::string & result, u_char * data, int len, int type);

