/*
 * Copyright (C) As Cloud, Inc.
 * Author   :   Hu Jin Bo
 * Data     :   01, AUG, 2021
 */

#include <string>
#include "conv.h"

/* tag_conv_base64 */
typedef struct ST_CONV_BASE64 tag_conv_base64;

/* ST_CONV_BASE64 */
struct ST_CONV_BASE64
{
	unsigned char	*dst;
	unsigned int	 dst_len;

	const char		*src;
	unsigned int	 src_len;

	const char		*basis;
	unsigned int	 padding;
};


/* template_base64_encode */
static void template_base64_encode(tag_conv_base64 *b64)
{
	char			*d, *s;
	unsigned int	 len;

	len = b64->src_len;
	s = const_cast<char *>(b64->src);
	d = (char *)b64->dst;

	while (len > 2) {
		*d++ = b64->basis[(s[0] >> 2) & 0x3f];
		*d++ = b64->basis[((s[0] & 3) << 4) | (s[1] >> 4)];
		*d++ = b64->basis[((s[1] & 0x0f) << 2) | (s[2] >> 6)];
		*d++ = b64->basis[s[2] & 0x3f];

		s += 3;
		len -= 3;
	}

	if (len) {
		*d++ = b64->basis[(s[0] >> 2) & 0x3f];

		if (len == 1) {

			*d++ = b64->basis[(s[0] & 3) << 4];
			if (b64->padding) {
				*d++ = '=';
			}

		}
		else {
			*d++ = b64->basis[((s[0] & 3) << 4) | (s[1] >> 4)];
			*d++ = b64->basis[(s[1] & 0x0f) << 2];
		}

		if (b64->padding) {
			*d++ = '=';
		}
	}

	b64->dst_len = d - (char *)b64->dst;
}


/* template_base64_decode */
static int template_base64_decode(tag_conv_base64 *b64)
{
	unsigned int         len;
	char				*d;
	unsigned char		 idx, *s;;

	for (len = 0; len < b64->src_len; len++) {
		if (b64->src[len] == '=') {
			break;
		}

		idx = (unsigned char)b64->src[len];
		if (b64->basis[idx] == 77) {
			return -1;
		}
	}

	if (len % 4 == 1) {
		return -1;
	}

	s = (unsigned char *)(const_cast<char *>(b64->src));
	d = (char *)b64->dst;

	while (len > 3) {

		*d++ = (char)(b64->basis[s[0]] << 2 | b64->basis[s[1]] >> 4);
		*d++ = (char)(b64->basis[s[1]] << 4 | b64->basis[s[2]] >> 2);
		*d++ = (char)(b64->basis[s[2]] << 6 | b64->basis[s[3]]);

		s += 4;
		len -= 4;
	}

	if (len > 1) {
		*d++ = (char)(b64->basis[s[0]] << 2 | b64->basis[s[1]] >> 4);
	}

	if (len > 2) {
		*d++ = (char)(b64->basis[s[1]] << 4 | b64->basis[s[2]] >> 2);
	}

	b64->dst_len = d - (char *)b64->dst;
	return b64->dst_len;
}


/* crypto_base64_encode */
int conv_base64_encode(unsigned char * dst, unsigned int len, const std::string & src)
{
	static char basis64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
	
	tag_conv_base64 b64;

	b64.dst = dst;
	b64.dst_len = len;

	b64.src = src.data();
	b64.src_len = src.size();

	b64.basis = basis64;
	b64.padding = 0;

	template_base64_encode(&b64);
	return static_cast<int>(strlen((char *)dst));
}

/* crypto_base64_encode */
int conv_base64_encode(unsigned char * bind, unsigned char *b64, unsigned int bin_len)
{
	static char		basis64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	unsigned int	i, j;
	unsigned char	c;

	for (i = 0, j = 0; i < bin_len; i += 3)
	{
		c = (bind[i] >> 2);
		c &= (unsigned char)0x3F;

		b64[j++] = basis64[(int)c];
		c = ((unsigned char)(bind[i] << 4)) & ((unsigned char)0x30);

		if (i + 1 >= bin_len)
		{
			b64[j++] = basis64[(int)c];
			b64[j++] = '=';
			b64[j++] = '=';
			break;
		}

		c |= ((unsigned char)(bind[i + 1] >> 4)) & ((unsigned char)0x0F);
		b64[j++] = basis64[(int)c];

		c = ((unsigned char)(bind[i + 1] << 2)) & ((unsigned char)0x3C);

		if (i + 2 >= bin_len)
		{
			b64[j++] = basis64[(int)c];
			b64[j++] = '=';
			break;
		}

		c |= ((unsigned char)(bind[i + 2] >> 6)) & ((unsigned char)0x03);
		b64[j++] = basis64[(int)c];

		c = ((unsigned char)bind[i + 2]) & ((unsigned char)0x3F);
		b64[j++] = basis64[(int)c];
	}

	b64[j] = '\0';
	return j;
}


/* conv_base64_decode */
int conv_base64_decode(unsigned char * dst, unsigned int len, const std::string & src)
{
	static char  basis64[] = {
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 62, 77, 77, 77, 63,
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77, 77, 77, 77, 77, 77,
		77,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 77, 77, 77, 77, 77,
		77, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 77, 77, 77, 77, 77,

		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77
	};

	tag_conv_base64 b64;

	b64.dst = dst;
	b64.dst_len = len;

	b64.src = src.data();
	b64.src_len = src.size();

	b64.basis = basis64;
	b64.padding = 0;
	
	return template_base64_decode(&b64);
};

