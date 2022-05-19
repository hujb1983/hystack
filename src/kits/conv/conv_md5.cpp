/*
 * Copyright (C) As Cloud, Inc.
 * Author   :   Hu Jin Bo
 * Data     :   01, AUG, 2021
 */

#include <string>
#include "conv.h"

/* tag_conv_md5 */
typedef struct ST_CONV_MD5	tag_conv_md5;

/* conv_md5 */
struct ST_CONV_MD5
{
	/* conv_md5 */
	const int			md5_byte16 = 1;
	const int			md5_char16 = 2;
	const int			md5_char32 = 3;

	unsigned long long	bytes;
	unsigned int		a, b, c, d;
	unsigned char		buffer[64];
};


void conv_md5_init(tag_conv_md5 * ctx);
void conv_md5_update(tag_conv_md5 * ctx, const void * data, unsigned int size);
void conv_md5_final(tag_conv_md5 * ctx, unsigned char result[16]);
void conv_md5_hax_to_char(unsigned char hex[16], unsigned char chs[32]);
const unsigned char * conv_md5_body(tag_conv_md5 * ctx, const unsigned char *data, unsigned int size);

#define F(x, y, z)  ((z) ^ ((x) & ((y) ^ (z))))
#define G(x, y, z)  ((y) ^ ((z) & ((x) ^ (y))))
#define H(x, y, z)  ((x) ^ (y) ^ (z))
#define I(x, y, z)  ((y) ^ ((x) | ~(z)))

#define STEP(f, a, b, c, d, x, t, s)                                \
		(a) += f((b), (c), (d)) + (x) + (t);                            \
		(a) = (((a) << (s)) | (((a) & 0xffffffff) >> (32 - (s))));      \
		(a) += (b)

#define SET(n)                                                      \
		(block[n] =                                                     \
		(uint32_t) p[n * 4] |                                           \
		((uint32_t) p[n * 4 + 1] << 8) |                                \
		((uint32_t) p[n * 4 + 2] << 16) |                               \
		((uint32_t) p[n * 4 + 3] << 24))

#define GET(n)      block[n]


void conv_md5_init(tag_conv_md5 * ctx)
{
	ctx->a = 0x67452301;
	ctx->b = 0xefcdab89;
	ctx->c = 0x98badcfe;
	ctx->d = 0x10325476;
	ctx->bytes = 0;

	memset(ctx->buffer, 0x0, 64);
}

const unsigned char * 
conv_md5_body(tag_conv_md5 * ctx, const unsigned char *data, unsigned int size)
{
	unsigned int		 a, b, c, d;
	unsigned int		 saved_a, saved_b, saved_c, saved_d;
	const unsigned char	*p;
	unsigned int		 block[16];

	p = data;

	a = ctx->a;
	b = ctx->b;
	c = ctx->c;
	d = ctx->d;

	do {
		saved_a = a;
		saved_b = b;
		saved_c = c;
		saved_d = d;

		/* Round 1 */

		STEP(F, a, b, c, d, SET(0), 0xd76aa478, 7);
		STEP(F, d, a, b, c, SET(1), 0xe8c7b756, 12);
		STEP(F, c, d, a, b, SET(2), 0x242070db, 17);
		STEP(F, b, c, d, a, SET(3), 0xc1bdceee, 22);
		STEP(F, a, b, c, d, SET(4), 0xf57c0faf, 7);
		STEP(F, d, a, b, c, SET(5), 0x4787c62a, 12);
		STEP(F, c, d, a, b, SET(6), 0xa8304613, 17);
		STEP(F, b, c, d, a, SET(7), 0xfd469501, 22);
		STEP(F, a, b, c, d, SET(8), 0x698098d8, 7);
		STEP(F, d, a, b, c, SET(9), 0x8b44f7af, 12);
		STEP(F, c, d, a, b, SET(10), 0xffff5bb1, 17);
		STEP(F, b, c, d, a, SET(11), 0x895cd7be, 22);
		STEP(F, a, b, c, d, SET(12), 0x6b901122, 7);
		STEP(F, d, a, b, c, SET(13), 0xfd987193, 12);
		STEP(F, c, d, a, b, SET(14), 0xa679438e, 17);
		STEP(F, b, c, d, a, SET(15), 0x49b40821, 22);

		/* Round 2 */

		STEP(G, a, b, c, d, GET(1), 0xf61e2562, 5);
		STEP(G, d, a, b, c, GET(6), 0xc040b340, 9);
		STEP(G, c, d, a, b, GET(11), 0x265e5a51, 14);
		STEP(G, b, c, d, a, GET(0), 0xe9b6c7aa, 20);
		STEP(G, a, b, c, d, GET(5), 0xd62f105d, 5);
		STEP(G, d, a, b, c, GET(10), 0x02441453, 9);
		STEP(G, c, d, a, b, GET(15), 0xd8a1e681, 14);
		STEP(G, b, c, d, a, GET(4), 0xe7d3fbc8, 20);
		STEP(G, a, b, c, d, GET(9), 0x21e1cde6, 5);
		STEP(G, d, a, b, c, GET(14), 0xc33707d6, 9);
		STEP(G, c, d, a, b, GET(3), 0xf4d50d87, 14);
		STEP(G, b, c, d, a, GET(8), 0x455a14ed, 20);
		STEP(G, a, b, c, d, GET(13), 0xa9e3e905, 5);
		STEP(G, d, a, b, c, GET(2), 0xfcefa3f8, 9);
		STEP(G, c, d, a, b, GET(7), 0x676f02d9, 14);
		STEP(G, b, c, d, a, GET(12), 0x8d2a4c8a, 20);

		/* Round 3 */

		STEP(H, a, b, c, d, GET(5), 0xfffa3942, 4);
		STEP(H, d, a, b, c, GET(8), 0x8771f681, 11);
		STEP(H, c, d, a, b, GET(11), 0x6d9d6122, 16);
		STEP(H, b, c, d, a, GET(14), 0xfde5380c, 23);
		STEP(H, a, b, c, d, GET(1), 0xa4beea44, 4);
		STEP(H, d, a, b, c, GET(4), 0x4bdecfa9, 11);
		STEP(H, c, d, a, b, GET(7), 0xf6bb4b60, 16);
		STEP(H, b, c, d, a, GET(10), 0xbebfbc70, 23);
		STEP(H, a, b, c, d, GET(13), 0x289b7ec6, 4);
		STEP(H, d, a, b, c, GET(0), 0xeaa127fa, 11);
		STEP(H, c, d, a, b, GET(3), 0xd4ef3085, 16);
		STEP(H, b, c, d, a, GET(6), 0x04881d05, 23);
		STEP(H, a, b, c, d, GET(9), 0xd9d4d039, 4);
		STEP(H, d, a, b, c, GET(12), 0xe6db99e5, 11);
		STEP(H, c, d, a, b, GET(15), 0x1fa27cf8, 16);
		STEP(H, b, c, d, a, GET(2), 0xc4ac5665, 23);

		/* Round 4 */

		STEP(I, a, b, c, d, GET(0), 0xf4292244, 6);
		STEP(I, d, a, b, c, GET(7), 0x432aff97, 10);
		STEP(I, c, d, a, b, GET(14), 0xab9423a7, 15);
		STEP(I, b, c, d, a, GET(5), 0xfc93a039, 21);
		STEP(I, a, b, c, d, GET(12), 0x655b59c3, 6);
		STEP(I, d, a, b, c, GET(3), 0x8f0ccc92, 10);
		STEP(I, c, d, a, b, GET(10), 0xffeff47d, 15);
		STEP(I, b, c, d, a, GET(1), 0x85845dd1, 21);
		STEP(I, a, b, c, d, GET(8), 0x6fa87e4f, 6);
		STEP(I, d, a, b, c, GET(15), 0xfe2ce6e0, 10);
		STEP(I, c, d, a, b, GET(6), 0xa3014314, 15);
		STEP(I, b, c, d, a, GET(13), 0x4e0811a1, 21);
		STEP(I, a, b, c, d, GET(4), 0xf7537e82, 6);
		STEP(I, d, a, b, c, GET(11), 0xbd3af235, 10);
		STEP(I, c, d, a, b, GET(2), 0x2ad7d2bb, 15);
		STEP(I, b, c, d, a, GET(9), 0xeb86d391, 21);

		a += saved_a;
		b += saved_b;
		c += saved_c;
		d += saved_d;

		p += 64;

	} while (size -= 64);

	ctx->a = a;
	ctx->b = b;
	ctx->c = c;
	ctx->d = d;

	return p;
}

void conv_md5_update(tag_conv_md5 * ctx, const void * data, unsigned int size)
{
	unsigned int  nUsed, nFree;

	nUsed = (unsigned int)(ctx->bytes & 0x3f);
	ctx->bytes += size;

	if (nUsed) {

		nFree = 64 - nUsed;

		if (size < nFree) {
			memcpy(&ctx->buffer[nUsed], data, size);
			return;
		}

		memcpy(&ctx->buffer[nUsed], data, nFree);
		data = (unsigned char *)data + nFree;
		size -= nFree;
		(void)conv_md5_body(ctx, ctx->buffer, 64);
	}

	if (size >= 64) {

		data = conv_md5_body(ctx, (unsigned char *)data, size & ~(unsigned int)0x3f);
		size &= 0x3f;
	}

	memcpy(ctx->buffer, data, size);
}


void conv_md5_final(tag_conv_md5 * ctx, unsigned char result[16])
{
	unsigned int  nUsed, nFree;

	nUsed = (unsigned int)(ctx->bytes & 0x3f);

	ctx->buffer[nUsed++] = 0x80;

	nFree = 64 - nUsed;

	if (nFree < 8) {

		memset(&ctx->buffer[nUsed], 0x0, nFree);
		(void)conv_md5_body(ctx, ctx->buffer, 64);

		nUsed = 0;
		nFree = 64;
	}

	memset(&ctx->buffer[nUsed], 0x0, nFree - 8);

	ctx->bytes <<= 3;
	ctx->buffer[56] = (unsigned char)ctx->bytes;
	ctx->buffer[57] = (unsigned char)(ctx->bytes >> 8);
	ctx->buffer[58] = (unsigned char)(ctx->bytes >> 16);
	ctx->buffer[59] = (unsigned char)(ctx->bytes >> 24);
	ctx->buffer[60] = (unsigned char)(ctx->bytes >> 32);
	ctx->buffer[61] = (unsigned char)(ctx->bytes >> 40);
	ctx->buffer[62] = (unsigned char)(ctx->bytes >> 48);
	ctx->buffer[63] = (unsigned char)(ctx->bytes >> 56);

	(void)conv_md5_body(ctx, ctx->buffer, 64);

	result[0] = (unsigned char)ctx->a;
	result[1] = (unsigned char)(ctx->a >> 8);
	result[2] = (unsigned char)(ctx->a >> 16);
	result[3] = (unsigned char)(ctx->a >> 24);
	result[4] = (unsigned char)ctx->b;
	result[5] = (unsigned char)(ctx->b >> 8);
	result[6] = (unsigned char)(ctx->b >> 16);
	result[7] = (unsigned char)(ctx->b >> 24);
	result[8] = (unsigned char)ctx->c;
	result[9] = (unsigned char)(ctx->c >> 8);
	result[10] = (unsigned char)(ctx->c >> 16);
	result[11] = (unsigned char)(ctx->c >> 24);
	result[12] = (unsigned char)ctx->d;
	result[13] = (unsigned char)(ctx->d >> 8);
	result[14] = (unsigned char)(ctx->d >> 16);
	result[15] = (unsigned char)(ctx->d >> 24);

	memset(ctx, 0x0, sizeof(*ctx));
}


void conv_md5_hax_to_char(unsigned char hex[16], unsigned char chs[32])
{
	int		H, L;

	for (int i = 0; i < 16; i++)
	{
		H = ((unsigned char)hex[i]) / 16;
		L = ((unsigned char)hex[i]) % 16;

		chs[2 * i] = (H > 9) ? char(H - 10 + 65) : char(H + 48);
		chs[2 * i + 1] = (L > 9) ? char(L - 10 + 65) : char(L + 48);
	}
}


void conv_md5_encode(char * _result, unsigned char _type, const std::string & _text)
{
	char	r[16] = { 0 };
	
	tag_conv_md5 ctx;
	
	conv_md5_init(&ctx);
	conv_md5_update(&ctx, _text.data(), _text.size());
	conv_md5_final(&ctx, (unsigned char *)r);

	if (_type == ctx.md5_char16) {

		memcpy(_result, r, 16);
	}
	else if (_type == ctx.md5_char16) {

		conv_md5_hax_to_char((unsigned char *)r, (unsigned char *)_result);

		memcpy(r, _result + 8, 16);
		memset(_result, 0x0, 32);
		memcpy(_result, r, 16);
	}
	else if (_type == ctx.md5_char32) {

		conv_md5_hax_to_char((unsigned char *)r, (unsigned char *)_result);
	}
}
