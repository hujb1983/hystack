/*
 * Copyright (C) As Cloud, Inc.
 * Author   :   Hu Jin Bo
 * Data     :   01, AUG, 2021
 */

#include <string>
#include "conv.h"

 /* tag_conv_base64 */
typedef struct ST_CONV_RC4	tag_conv_rc4;

/* ST_CONV_BASE64 */
struct ST_CONV_RC4
{
	int				 S[256];
	int				 T[256];

	const char		*key;
	unsigned int	 size;

	unsigned char	*chs;
	unsigned int	 chs_len;

	unsigned char	 *hex;
	unsigned int	 hex_len;
};

 /* conv_rc4 */
void conv_rc4_char_to_hex(tag_conv_rc4 *ctx);
void conv_rc4_hex_to_char(tag_conv_rc4 *ctx);
char *conv_rc4_to_rc4(tag_conv_rc4 *ctx);

void conv_rc4_encode(unsigned char * hax, unsigned int haxLen,
		unsigned char * chs, unsigned int chsLen, const std::string & szKey);
void conv_rc4_decode(unsigned char * chs, unsigned int chsLen,
		unsigned char * hax, unsigned int haxLen, const std::string & szKey);


void conv_rc4_char_to_hex(tag_conv_rc4 *ctx)
{
	int		H, L;

	for (int i = 0; i < (int)ctx->chs_len; i++)
	{
		H = ((unsigned char)ctx->chs[i]) / 16;
		L = ((unsigned char)ctx->chs[i]) % 16;

		ctx->hex[2 * i] = (H > 9) ? char(H - 10 + 65) : char(H + 48);
		ctx->hex[2 * i + 1] = (L > 9) ? char(L - 10 + 65) : char(L + 48);
	}
}


//16 -> char 
void conv_rc4_hex_to_char(tag_conv_rc4 *ctx)
{
	int	H, L;

	for (int i = 0; i < (int)ctx->hex_len / 2; i++)
	{
		H = ((unsigned char)ctx->hex[2 * i]);
		L = ((unsigned char)ctx->hex[2 * i + 1]);

		H = (H >= 65) ? (H - 65 + 10) : (H - 48);
		L = (L >= 65) ? (L - 65 + 10) : (L - 48);

		ctx->chs[i] = char(H * 16 + L);
	}
}


char * conv_rc4_to_rc4(tag_conv_rc4 *ctx)
{
	int i, j, k;
	int	tmp, count = ctx->size;
	int len;

	int * S = ctx->S;
	int * T = ctx->T;

	for (int i = 0; i < 256; i++)
	{
		S[i] = i;
		tmp = i % count;
		T[i] = ctx->key[tmp];
	}

	i = 0;
	j = 0;

	for (i = 0; i < 256; i++)
	{
		j = (j + S[i] + T[i]) % 256;

		tmp = S[j];
		S[j] = S[i];
		S[i] = tmp;
	}

	len = ctx->chs_len;

	i = 0;
	j = 0;

	for (int p = 0; p < len; p++)
	{
		i = (i + 1) % 256;
		j = (j + S[i]) % 256;

		tmp = S[j];
		S[j] = S[i];
		S[i] = tmp;

		k = S[(S[i] + S[j]) % 256];
		ctx->chs[p] = ctx->chs[p] ^ k;
	}
	return (char *)ctx->chs;
}


/* conv_rc4_encode */
void conv_rc4_encode(u_char * hex, u_int hex_len, u_char * chs, u_int chs_len, const std::string &key)
{
	tag_conv_rc4	ctx;

	ctx.hex = hex;
	ctx.hex_len = hex_len;

	ctx.chs = chs;
	ctx.chs_len = chs_len;

	std::string tmp = (char *)chs;

	ctx.key = key.data();
	ctx.size = key.size();

	conv_rc4_to_rc4(&ctx);
	conv_rc4_char_to_hex(&ctx);
}


/* conv_rc4_decode */
void conv_rc4_decode(u_char * chs, u_int chs_len, u_char * hex, u_int hex_len, const std::string & key)
{
	tag_conv_rc4	ctx;

	ctx.hex = hex;
	ctx.hex_len = hex_len;

	ctx.chs = chs;
	ctx.chs_len = chs_len;

	std::string tmp = (char *)chs;

	ctx.key = key.data();
	ctx.size = key.size();

	conv_rc4_hex_to_char(&ctx);
	conv_rc4_to_rc4(&ctx);
}
