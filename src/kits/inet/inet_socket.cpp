/*
 * Copyright (C) Asynet, Inc.
 * Data     :   01, AUG, 2021
 */

#include <string>
#include <memory>

#include "inet_socket.h"
#include "inet_parse.h"

 // openssl
#if (OPENSSL)
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/conf.h>
#endif

 // typedef struct
typedef struct ST_SOCKET	tag_socket;
typedef struct ST_ADDRESS	tag_address;


/* ADDRESS_LIST */
using ADDRESS_LIST = std::vector<tag_address>;


/* ST_SOCKET */
struct ST_ADDRESS
{
	unsigned int		vaild;
	std::string			address;
};

/* ST_SOCKET */
struct ST_SOCKET
{
	int					family;
	int					type;
	int					protocol;

	int					fd;
	unsigned short		port;

	unsigned int		ssl_on;

#if (OPENSSL)
	SSL_CTX			   *ctx;
	SSL				   *ssl;
	unsigned int		pending;
#endif

	std::string			domain;
	ADDRESS_LIST		addr_list;

	const char*			recv_buff;
	unsigned int		recv_buff_size;
	unsigned int		recv_buff_sum;
	unsigned int		recv_buff_max;

	const char*			send_buff;
	unsigned int		sent_buff_size;
	unsigned int		sent_buff_sum;
	unsigned int		sent_buff_max;

	unsigned int		error;
};

 /* inet_socket */
class inet_socket
{
public:
	inet_socket() {

	}

	virtual ~inet_socket() {

	}

public:
	virtual int  init(tag_socket * sock);
	virtual void close(tag_socket * sock);

public:
	virtual int  init_ssl(tag_socket * sock);
	virtual int  create_ctx(tag_socket * sock);
	virtual int  create_ssl(tag_socket * sock);
	virtual void release_ssl(tag_socket * sock);

public:
	virtual int nslookup(tag_socket * sock);
	virtual int connect(tag_socket * sock);

public:
	virtual int recv(tag_socket * sock);
	virtual int send(tag_socket * sock);

public:
	virtual int recv_tcp(tag_socket * sock);
	virtual int send_tcp(tag_socket * sock);
	virtual int recv_ssl(tag_socket * sock);
	virtual int send_ssl(tag_socket * sock);
};


/* inet_socket::init */
int inet_socket::init(tag_socket * sock)
{
	sock->fd = 0;
	sock->fd = ::socket(sock->family, sock->type, sock->protocol);

	if (sock->fd == (int)INVALID_SOCKET) {
		return -1;
	}

	/* openssl */
	sock->ssl_on = 0;
	sock->pending = 0;
	return 0;
}

/* inet_socket::init */
void inet_socket::close(tag_socket * sock)
{
	release_ssl(sock);

#if (WINX)
	::closesocket(sock->fd);
#else
	::close(sock->fd);
#endif

	sock->fd = -1;
}

/* inet_socket::init */
int inet_socket::init_ssl(tag_socket * sock)
{
#if (OPENSSL)
	sock->ctx = nullptr;
	sock->ssl = nullptr;


	#if OPENSSL_VERSION_NUMBER >= 0x10100003L

		if (OPENSSL_init_ssl(OPENSSL_INIT_LOAD_CONFIG, NULL) == 0) {
			ERR_clear_error();
			return -1;
		}

		ERR_clear_error();

	#else
		OPENSSL_config(nullptr);
		SSL_library_init();
		SSL_load_error_strings();
		OpenSSL_add_all_algorithms();
	#endif

	create_ctx(sock);
	create_ssl(sock);
#endif

	return 0;
}


/* inet_socket::close_ssl */
int inet_socket::create_ctx(tag_socket * sock)
{
#if (OPENSSL)
	const SSL_METHOD * method = SSLv23_client_method();

	sock->ctx = SSL_CTX_new(method);
	if (sock->ctx == nullptr) {
		return -1;
	}
#endif

	return 0;
}


/* inet_socket::close_ssl */
int inet_socket::create_ssl(tag_socket * sock)
{
#if (OPENSSL)
	int  ret = 0;

	sock->ssl = SSL_new(sock->ctx);
	if (sock->ssl == nullptr) {
		return -1;
	}

	SSL_set_fd(sock->ssl, sock->fd);
	SSL_set_connect_state(sock->ssl);

	ret = SSL_connect(sock->ssl);
	if (ret == -1) {
		release_ssl(sock);
		return -1;
	}
#endif

	return 0;
}

/* inet_socket::close_ssl */
void inet_socket::release_ssl(tag_socket * sock)
{
#if (OPENSSL)
	if (sock->ssl_on == 1)
	{
		if (sock->ssl != nullptr) {
			SSL_shutdown(sock->ssl);
			SSL_free(sock->ssl);
			sock->ssl = nullptr;
		}

		if (sock->ctx != nullptr) {
			SSL_CTX_free(sock->ctx);
			sock->ctx = nullptr;
		}
	}
#endif
}

/* inet_socket::nslookup */
int inet_socket::nslookup(tag_socket * sock)
{
	struct hostent	*he;
	std::string		 ip_addr;

	if ((he = gethostbyname(sock->domain.data())) == nullptr) {
		return -1;
	}

	for (int i = 0; he->h_addr_list[i]; i++) 
	{
		ip_addr = inet_ntoa(*(struct in_addr*)he->h_addr_list[i]);
		
		tag_address address;
		address.vaild = 1;
		address.address = ip_addr;
		sock->addr_list.emplace_back(address);
	}

	return 0;
}


/* inet_socket::connect */
int inet_socket::recv(tag_socket * sock)
{
	return 0;
}


/* inet_socket::connect */
int inet_socket::send(tag_socket * sock)
{
	return 0;
}

/* inet_socket::connect */
int inet_socket::connect(tag_socket * sock)
{
	int ret = 0;

	if (sock->family == AF_INET)
	{
		for (int i = 0; i < (int)sock->addr_list.size(); i++)
		{
			auto addr = sock->addr_list[i];

			struct sockaddr_in _sockaddr;
			_sockaddr.sin_family = AF_INET;
			_sockaddr.sin_port = htons(sock->port);
			_sockaddr.sin_addr.s_addr = inet_addr(addr.address.data());

			ret = ::connect(sock->fd, (struct sockaddr *)&_sockaddr, sizeof(_sockaddr));
			if (ret == SOCKET_ERROR) {
				continue;
			}

			addr.vaild = 1;
			break;
		}

		if (sock->ssl_on == 1) {
			this->init_ssl(sock);
		}

	}
	else if (sock->family == AF_UNIX) {

#if (!WINX)
		struct sockaddr_un un;
		memset(&un, 0, sizeof(un));

		un.sun_family = AF_UNIX;
		strcpy(un.sun_path, sock->domain.data());

		int	len = offsetof(struct sockaddr_un, sun_path) + strlen(sock->domain.data());
		ret = ::connect(sock->fd, (struct sockaddr *)&un, len);
#endif

	}

	return ret;
}


/* inet_socket::recv */
int inet_socket::recv_tcp(tag_socket * sock)
{
	const char * ptr = sock->recv_buff;
	unsigned int max_size = sock->recv_buff_max;

	int ready = ::recv(sock->fd, (char *)ptr, max_size, 0);
	if (ready > 0) {
		sock->recv_buff_size = ready;
		sock->recv_buff_sum += ready;
		return  ready;
	}

	return  ready;
}

/* inet_socket::send */
int inet_socket::send_tcp(tag_socket * sock)
{
	if (sock->fd == -1) {
		return sock->fd;
	}

	int sent = 0, ready = 0;

	const char		*ptr = sock->send_buff;
	unsigned int	 send_size = sock->sent_buff_size;

	while (sent < (int)sock->sent_buff_size)
	{
		ready = ::send(sock->fd, (char *)(ptr + sent), send_size - sent, 0);
		if (ready == -1) {
			return -1;
		}

		sent += ready;
		sock->sent_buff_sum += ready;
	}

	return (int)sent;
}


/* inet_socket::recv_ssl */
int inet_socket::recv_ssl(tag_socket * sock)
{
	int ready = 0;

#if (OPENSSL)

	const char * ptr = sock->recv_buff;
	unsigned int len = sock->recv_buff_max;

	ready = SSL_read(sock->ssl, (char *)ptr, len);
	if (SSL_pending(sock->ssl) > 0) {

		sock->error = SSL_get_error(sock->ssl, ready);
		if (sock->error != SSL_ERROR_NONE) {
			if (sock->error == SSL_ERROR_ZERO_RETURN) {
				return sock->error;
			}
			return sock->error;
		}

		sock->pending = 1;
		if (ready > 0) {
			return ready;
		}
	}
#endif

	sock->pending = 0;
	return ready;
}


/* inet_socket::send_ssl */
int inet_socket::send_ssl(tag_socket * sock)
{
	int				err = 0;

#if (OPENSSL)
	int				ready = 0;
	int				err_ssl;
	std::string		err_str;

	if (sock->fd == -1) {
		return sock->fd;
	}

	if (sock->ssl == nullptr) {
		return -1;
	}

	const char * ptr = sock->send_buff;
	unsigned int len = sock->sent_buff_size;

	ready = SSL_write(sock->ssl, (char *)(ptr), len);
	if (ready > 0) {
		sock->sent_buff_sum = ready;
		return ready;
	}

	err_ssl = SSL_get_error(sock->ssl, ready);
	if (err_ssl == SSL_ERROR_ZERO_RETURN) {
		err_ssl = SSL_ERROR_SYSCALL;
	}

	err = (err_ssl == SSL_ERROR_SYSCALL) ? errno : 0;

	// error;
	err_str = "SSL_get_error: ";
	err_str += std::to_string(err);

	if (err_ssl == SSL_ERROR_WANT_WRITE) {
		return SSL_ERROR_WANT_WRITE;
	}

	if (err_ssl == SSL_ERROR_WANT_READ) {
		return SSL_ERROR_WANT_READ;
	}
#endif

	return err;
}

/* inet_socket_new */
inet_handler* inet_socket_new(int type)
{
	tag_socket * ptr = new tag_socket;

	if (INET_SOCKET_TCP == type) {

		ptr->family = AF_INET;
		ptr->type = SOCK_STREAM;
		ptr->protocol = 0;
	}
	else if (INET_SOCKET_UNX == type) {
		ptr->family = AF_UNIX;
		ptr->type = SOCK_STREAM;
		ptr->protocol = 0;
	}

	ptr->fd = 0;
	return (inet_handler *)ptr;
}

/* inet_socket_delete */
void inet_socket_delete(inet_handler *hc)
{
	tag_socket * ptr = (tag_socket *) hc;
	if (ptr == nullptr) {
		return;
	}

	std::shared_ptr<inet_socket> sock;
	sock = std::make_shared<inet_socket>();
	sock->close(ptr);

	if (hc != nullptr) {
		delete hc;
	}

	hc = nullptr;
}

/* inet_socket_new */
int inet_socket_connect(inet_handler *hc, const char * api)
{
	tag_socket * ptr = (tag_socket *) hc;
	if (ptr == nullptr) {
		return -1;
	}

	std::shared_ptr<inet_socket> sock;
	sock = std::make_shared<inet_socket>();
	sock->init(ptr);

	if (ptr->family == AF_INET) {
		
		inet_parse_uri uri(api);
		if ( strcmp(uri.protocol.data(), "https") == 0 
			|| strcmp(uri.protocol.data(), "wss") == 0) {
			ptr->ssl_on = 1;
		}

		ptr->domain = uri.host;
		ptr->port = uri.port;
		sock->nslookup(ptr);
	}
	else {

		ptr->domain = api;
		ptr->port = 0;
	}

	return sock->connect(ptr);
}

/* inet_socket_new */
int inet_socket_select(inet_handler *hc, unsigned int timeout)
{
	tag_socket * ptr = (tag_socket *)hc;
	if (ptr == nullptr) {
		return -1;
	}

	int ready = 0;
	int fd_ = ptr->fd;

	if (ptr->ssl_on == 1 && ptr->pending == 1) {
		return fd_;
	}

	fd_set read_set_;
	FD_ZERO(&read_set_);
	FD_SET(fd_, &read_set_);

	unsigned int times = timeout / 1000;
	times = (times > 1) ? (times) : (1);

	struct timeval tv;
	tv.tv_sec = times;
	tv.tv_usec = 0;

	ready = select(fd_ + 1, &read_set_, nullptr, nullptr, &tv);
	if (ready < 0) {
		return -1;
	}

	if (ready == 0) {
		return ready;
	}
	return fd_;
}

/* inet_socket_new */
int inet_socket_recv(inet_handler *hc, unsigned char * data, unsigned short size)
{
	tag_socket * ptr = (tag_socket *)hc;
	if (ptr == nullptr) {
		return -1;
	}

	std::shared_ptr<inet_socket> sock;
	sock = std::make_shared<inet_socket>();

	ptr->recv_buff = (const char *)data;
	ptr->recv_buff_size = size;
	ptr->recv_buff_max = size;

	if (ptr->ssl_on == 1) {
		return sock->recv_ssl(ptr);
	}

	return sock->recv_tcp(ptr);
}


/* inet_socket_new */
int inet_socket_send(inet_handler *hc, unsigned char * data, unsigned short size)
{
	tag_socket * ptr = (tag_socket *)hc;
	if (ptr == nullptr) {
		return -1;
	}

	std::shared_ptr<inet_socket> sock;
	sock = std::make_shared<inet_socket>();

	ptr->send_buff = (const char *)data;
	ptr->sent_buff_size = size;
	ptr->sent_buff_max = size;

	if (ptr->ssl_on == 1) {
		return sock->send_ssl(ptr);
	}

	return sock->send_tcp(ptr);
}
