/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#pragma once

struct ST_STRING {
	unsigned int	 len;
	unsigned char	*data;
};

#define init_string(str)		{ sizeof(str) - 1, (unsigned char *)str }
#define init_string_null		{ 0, nullptr }

#define INT32_LEN				(sizeof("-2147483648") - 1)
#define INT64_LEN				(sizeof("-9223372036854775808") - 1)

#define INT_LEN					INT64_LEN
#define MAX_INT_VALUE			9223372036854775807

#define OFF_LEN					(sizeof("-9223372036854775807") - 1)
#define MAX_OFF_VALUE			9223372036854775807

#define LF						(unsigned char) '\n'
#define CR						(unsigned char) '\r'
#define CRLF					"\r\n"

#define TYPE_INT32_LEN			(sizeof("-2147483648") - 1)
#define TYPE_INT64_LEN			(sizeof("-9223372036854775808") - 1)

#define str_tolower(c)			(u_char) ((c >= 'A' && c <= 'Z') ? (c | 0x20) : c)
#define str_toupper(c)			(u_char) ((c >= 'a' && c <= 'z') ? (c & ~0x20) : c)

/* string_cmp */
int str_case_cmp(u_char *s1, u_char *s2);
int str_case_cmp_null(u_char *s1, u_char *s2, u_int n);

/* hex_toi */
int hex_toi(const char s[], int start, int len);

#define str_memcpy(dst, src, n)   (void) memcpy(dst, src, n)
#define str_cpymem(dst, src, n)   (((u_char *) memcpy(dst, src, n)) + (n))
