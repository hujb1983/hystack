/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <algorithm>

#include "conv.h"
using namespace std;


/* conv_rsa */
class conv_rsa
{
public:
	conv_rsa() {};
	~conv_rsa() {};

public:
	int e;
	int	d;
	int n;

public:
	int  gcd(int a, int b);
	void gcdext(long long a, long long b, long long& d, long long& x, long long& y);
	int  primarity(int a, int i);
	int  modular_exponention(int a, int b, int n);
	int  modular_inverse(int a, int b);
	void key_generation();
};


/* conv_rsa::gcd */
int conv_rsa::gcd(int a, int b)
{
	int c = 0;
	if (a < b) {
		swap(a, b);
	}

	c = b;
	do
	{
		b = c;
		c = a % b;
		a = b;
	} while (c != 0);

	return b;
}


/* conv_rsa::primarity */
int conv_rsa::primarity(int a, int i)
{
	int flag = 0;
	for (; a < i; a++)
	{
		if (i%a == 0)
		{
			flag = 1;
			break;
		}
	}

	if (flag) {
		return 0;
	}
	return 1;
}


/* conv_rsa::modular_exponention */
int conv_rsa::modular_exponention(int a, int b, int n)
{
	int y = 1;

	while (b != 0)
	{
		if (b & 1) {
			y = (y*a) % n;
		}

		a = (a*a) % n;
		b = b >> 1;
	}

	return y;
}


/* conv_rsa::gcdext */
void conv_rsa::gcdext(long long a, long long b, long long& d, long long& x, long long& y)
{
	if (!b)
	{
		d = a;
		x = 1;
		y = 0;
	}
	else
	{
		gcdext(b, a%b, d, y, x);
		y -= x * (a / b);
	}
}


/* conv_rsa::modular_inverse */
int conv_rsa::modular_inverse(int a, int b)
{
	long long d, x, y;
	gcdext(a, b, d, x, y);
	return d == 1 ? (x + b) % b : -1;
}


/* conv_rsa::key_generation */
void conv_rsa::key_generation()
{
	int p, q;
	int phi_n;

	do
	{
		do
			p = rand();
		while (p % 2 == 0);

	} while (!primarity(2, p));

	do
	{
		do
			q = rand();
		while (q % 2 == 0);

	} while (!primarity(2, q));

	n = p * q;
	phi_n = (p - 1) * (q - 1);

	do {
		e = rand() % (phi_n - 2) + 2; // 1 < e < phi_n
	} while (gcd(e, phi_n) != 1);

	d = modular_inverse(e, phi_n);
}


/* conv_rsa_encode */
void conv_rsa_encode(int e, int d, int n, int value, FILE* out)
{
	conv_rsa rsa;
	rsa.e = e;
	rsa.d = d;
	rsa.n = n;

	rsa.modular_exponention(value, e, n);
}


/* conv_rsa_decode */
void conv_rsa_decode(int e, int d, int n, int value, FILE* out)
{
	conv_rsa rsa;
	rsa.e = e;
	rsa.d = d;
	rsa.n = n;

	rsa.modular_exponention(value, d, n);
}

