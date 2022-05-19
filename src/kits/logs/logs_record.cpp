/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, AUG, 2021
 */
#include <mutex>
#include <string>
#include <fstream>
#include <chrono>
using namespace std;
using namespace chrono;

#include <logs/logs.h>
#include "logs_record.h"

/*
* The digits table is used to look up for number within 100.
* Each two character corresponds to one digit and ten digits.
*/
static constexpr char _digits_table[200] =
{
	'0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0',
	'7', '0', '8', '0', '9', '1', '0', '1', '1', '1', '2', '1', '3', '1', '4',
	'1', '5', '1', '6', '1', '7', '1', '8', '1', '9', '2', '0', '2', '1', '2',
	'2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7', '2', '8', '2', '9',
	'3', '0', '3', '1', '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3',
	'7', '3', '8', '3', '9', '4', '0', '4', '1', '4', '2', '4', '3', '4', '4',
	'4', '5', '4', '6', '4', '7', '4', '8', '4', '9', '5', '0', '5', '1', '5',
	'2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
	'6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6',
	'7', '6', '8', '6', '9', '7', '0', '7', '1', '7', '2', '7', '3', '7', '4',
	'7', '5', '7', '6', '7', '7', '7', '8', '7', '9', '8', '0', '8', '1', '8',
	'2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9',
	'9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9',
	'7', '9', '8', '9', '9'
};

static unsigned int _u2a(unsigned long long number, char *to)
{
	char			 buf[24];
	char			*p = buf;
	unsigned int	 length = 0;

	while (number >= 100)
	{
		const unsigned idx = (number % 100) << 1;
		number /= 100;
		*p++ = _digits_table[idx + 1];
		*p++ = _digits_table[idx];
	}

	if (number < 10) {
		*p++ = (char)number + '0';
	}
	else {
		const unsigned char idx = (unsigned char)number << 1;
		*p++ = _digits_table[idx + 1];
		*p++ = _digits_table[idx];
	}

	length = p - buf;

	do {
		*to++ = *--p;
	} while (p != buf);
	*to = '\0';

	return(length);
}

static size_t _i2a(int64_t number, char *to)
{
	uint64_t	n = static_cast<uint64_t>(number);
	size_t		sign = 0;
	if (number < 0)
	{
		*to++ = '-';
		n = ~n + 1;
		sign = 1;
	}

	return(_u2a(n, to) + sign);
}

logs_record::logs_record(void * log, int level)
	:log_(log), level_(level) {
}

logs_record::~logs_record() {
	logs_print((tag_log *)log_, level_, line_);
}

logs_record& logs_record::operator<<(char arg) {
	append(&arg, 1);
	return(*this);
}

logs_record& logs_record::operator<<(short arg) {
	char	tmp[8];
	size_t	len = _i2a(arg, tmp);
	append(tmp, len);
	return(*this);
}

logs_record& logs_record::operator<<(unsigned short arg) {
	char	tmp[8];
	size_t	len = _u2a(arg, tmp);
	append(tmp, len);
	return(*this);
}

logs_record& logs_record::operator<<(int arg) {
	char	tmp[12];
	size_t	len = _i2a(arg, tmp);
	append(tmp, len);
	return(*this);
}

logs_record& logs_record::operator<<(unsigned int arg) {
	char	tmp[12];
	size_t	len = _u2a(arg, tmp);
	append(tmp, len);
	return(*this);
}

logs_record& logs_record::operator<<(long long arg) {
	char	tmp[24];
	size_t	len = _i2a(arg, tmp);
	append(tmp, len);
	return(*this);
}

logs_record& logs_record::operator<<(unsigned long long arg) {
	char	tmp[24];
	size_t	len = _u2a(arg, tmp);
	append(tmp, len);
	return(*this);
}

logs_record& logs_record::operator<<(double arg) {
	append(std::to_string(arg).data());
	return(*this);
}

logs_record& logs_record::operator<<(const char *arg) {
	append(arg);
	return(*this);
}

logs_record& logs_record::operator<<(const std::string &arg) {
	append(arg.data(), arg.length());
	return(*this);
}

void logs_record::append(const char *data, unsigned int n) {
	line_ += data;
	line_ += " ";
	count_ += static_cast<unsigned int>(n);
}

void logs_record::append(const char *data) {
	append(data, (unsigned int)strlen(data));
}
