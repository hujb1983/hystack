/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#include "core_config.h"
#include "core_string.h"


/* str_case_cmp */
int str_case_cmp(u_char *s1, u_char *s2)
{
	u_int  c1, c2;

	for (;; ) {
		c1 = (u_int)*s1++;
		c2 = (u_int)*s2++;

		c1 = (c1 >= 'A' && c1 <= 'Z') ? (c1 | 0x20) : c1;
		c2 = (c2 >= 'A' && c2 <= 'Z') ? (c2 | 0x20) : c2;

		if (c1 == c2) {

			if (c1) {
				continue;
			}

			return 0;
		}

		return c1 - c2;
	}
}


/* str_case_cmp_null */
int str_case_cmp_null(u_char *s1, u_char *s2, u_int n)
{
	u_int  c1, c2;

	while (n) {
		c1 = (u_int)*s1++;
		c2 = (u_int)*s2++;

		c1 = (c1 >= 'A' && c1 <= 'Z') ? (c1 | 0x20) : c1;
		c2 = (c2 >= 'A' && c2 <= 'Z') ? (c2 | 0x20) : c2;

		if (c1 == c2) {

			if (c1) {
				n--;
				continue;
			}

			return 0;
		}

		return c1 - c2;
	}

	return 0;
}


/* hex_toi */
int hex_toi(const char s[], int start, int len)
{
	int i, j;
	int n = 0;
	if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
		i = 2;
	}
	else {
		i = 0;
	}

	i += start;
	j = 0;

	for (; (s[i] >= '0' && s[i] <= '9')
		|| (s[i] >= 'a' && s[i] <= 'f') || (s[i] >= 'A' && s[i] <= 'F'); ++i)
	{
		if (j >= len) {
			break;
		}
		if (str_tolower(s[i]) > '9') {
			n = 16 * n + (10 + str_tolower(s[i]) - 'a');
		}
		else {
			n = 16 * n + (str_tolower(s[i]) - '0');
		}

		j++;
	}

	return n;
}
