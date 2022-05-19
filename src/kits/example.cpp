/*
 * Copyright (C) As Cloud, Inc.
 * Data     :   01, FEB, 2021
 */

#include "conv/conv.h"

#if (WINX)
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")
#endif


/* windows_socket_initialize */
unsigned int windows_socket_initialize()
{
#if (WINX)
	WORD	wVersionRequested;
	WSADATA wsaData;
	int		err;

	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);	//load win socket
	if (err != 0) {
		exit(0);
	}
#endif

	return 0;
};


 /* main */
int main(int argc, char *const *argv)
{
	windows_socket_initialize();

	unsigned char	dst[100] = { "123123123" };
	unsigned int	dst_len = 100;
	std::string		src;

	conv_base64_encode(dst, dst_len, src);

	return 0;
}
