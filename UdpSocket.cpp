#include "UdpSocket.h"

#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

UdpSocket::UdpSocket() : fd(-1)
{
}
UdpSocket::UdpSocket(int port) : fd(-1), m_port(port)
{
}
UdpSocket::~UdpSocket()
{
	if (fd != -1)
		close();
}

bool UdpSocket::init()
{
	if ((fd = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return false;

	return true;
}
bool UdpSocket::bind(int port)
{
	m_port = port;
	return bind();
}
bool UdpSocket::bind()
{
	struct sockaddr_in myaddr;

	memset((char*)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(m_port);

	if (::bind(fd, (struct sockaddr*)&myaddr, sizeof(myaddr)) < 0)
		return false;

	return true;
}
void UdpSocket::close()
{
	::close(fd);
}

bool UdpSocket::send(const char* ip, uint16_t port, const void* data, int len)
{
	struct sockaddr_in remaddr;
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &(remaddr.sin_addr));

	if (sendto(fd, (const char*)data, len, 0, (struct sockaddr*)&remaddr, sizeof(remaddr)) < 0) {
		return false;
	} else {
		return true;
	}
}
int UdpSocket::recv(const char ip[20], uint16_t& port, void* data, int len, uint32_t timeout)
{
	fd_set fds;

	struct timeval timeout_val = {0};
	struct timeval *timeout_ptr;
	if (timeout == 0xffffffff) {
		timeout_ptr = 0;
	} else {
		timeout_val.tv_sec = timeout / 1000;
		timeout_val.tv_usec = (timeout - timeout_val.tv_sec * 1000) * 1000;
		timeout_ptr = &timeout_val;
	}

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	// checking for new packet
	int res = select(fd + 1, &fds, 0, 0, timeout_ptr);
	if (res == -1) {
		if (errno != EINTR) {
			return -1;
		}
	} else {
		if (FD_ISSET(fd, &fds)) {
			struct sockaddr_in remaddr;
			socklen_t addrlen = sizeof(remaddr);
			int recvlen = recvfrom(fd, data, len, 0, (struct sockaddr*)&remaddr, &addrlen);

			inet_ntop(AF_INET, &(remaddr.sin_addr), (char*)ip, 20);
			port = ntohs(remaddr.sin_port);

			return recvlen;
		}
	}

	return 0;
}
