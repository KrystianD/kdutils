#include "UdpSocket.h"

#if defined(WIN32) || defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#define socklen_t int
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#endif

#include <kdutils.h>

UdpSocket::UdpSocket() : m_listener(0), m_sockfd(-1), m_timeout(10)
{
}
UdpSocket::UdpSocket(int port) : m_listener(0), m_sockfd(-1), m_port(port), m_timeout(10)
{
}
UdpSocket::~UdpSocket()
{
	if (m_sockfd != -1)
		close();
}

bool UdpSocket::init()
{
	if ((m_sockfd = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		m_lastErrorStr = string("cannot create socket: ") + getErrnoString();
		return false;
	}
	
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
	
	if (::bind(m_sockfd, (struct sockaddr*)&myaddr, sizeof(myaddr)) < 0)
	{
		m_lastErrorStr = string("bind failed: ") + getErrnoString();
		return false;
	}
	
	return true;
}
void UdpSocket::close()
{
#if defined(WIN32) || defined(_WIN32)
	closesocket(m_sockfd);
#else
	::close(m_sockfd);
#endif
}
bool UdpSocket::process()
{
	timeval tv;
	fd_set fds;
	
	tv.tv_sec = 0;
	tv.tv_usec = m_timeout * 1000;
	
	FD_ZERO(&fds);
	FD_SET(m_sockfd, &fds);
	
	// checking for new incoming connections
	int res = select(m_sockfd + 1, &fds, 0, 0, &tv);
	if (res == -1)
	{
		if (errno != EINTR)
		{
			m_lastErrorStr = string("select failed: ") + getErrnoString();
			return false;
		}
	}
	else
	{
		if (FD_ISSET(m_sockfd, &fds))
		{
			struct sockaddr_in remaddr;
			socklen_t addrlen = sizeof(remaddr);
			char buf[64*1024];
			int recvlen = recvfrom(m_sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&remaddr, &addrlen);
			
			// char ip[INET_ADDRSTRLEN];
			char ip[100];
			inet_ntop(AF_INET, &(remaddr.sin_addr), ip, sizeof(ip));
			
			if (m_listener)
				m_listener->onEthernetDataReceived(ip, buf, recvlen);
		}
	}
	
	return true;
}

bool UdpSocket::send(const string& ip, uint16_t port, const void* data, int len)
{
	struct sockaddr_in remaddr;
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(port);
#if defined(WIN32) || defined(_WIN32)
	remaddr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
#else
	inet_pton(AF_INET, ip.c_str(), &(remaddr.sin_addr));
#endif
	
	// buffer.print();
	if (sendto(m_sockfd, (const char*)data, len, 0, (struct sockaddr*)&remaddr, sizeof(remaddr)) < 0)
	{
		return false;
		// printf("send fail\n");
	}
	else
	{
		return true;
		// printf("send OK\n");
	}
}
int UdpSocket::read(string& ip, uint16_t& port, void* data, int len, uint32_t timeout)
{
	timeval tv;
	fd_set fds;
	
	struct timeval timeout_val = {0};
	struct timeval *timeout_ptr;
	if (timeout == 0xffffffff)
	{
		timeout_ptr = 0;
	}
	else
	{
		timeout_val.tv_sec = timeout / 1000;
		timeout_val.tv_usec = (timeout - timeout_val.tv_sec * 1000) * 1000;
		timeout_ptr = &timeout_val;
	}
	
	FD_ZERO(&fds);
	FD_SET(m_sockfd, &fds);
	
	// checking for new incoming connections
	int res = select(m_sockfd + 1, &fds, 0, 0, timeout_ptr);
	if (res == -1)
	{
		if (errno != EINTR)
		{
			m_lastErrorStr = string("select failed: ") + getErrnoString();
			return -1;
		}
	}
	else
	{
		if (FD_ISSET(m_sockfd, &fds))
		{
			struct sockaddr_in remaddr;
			socklen_t addrlen = sizeof(remaddr);
			int recvlen = recvfrom(m_sockfd, data, len, 0, (struct sockaddr*)&remaddr, &addrlen);
			
			// char ip[INET_ADDRSTRLEN];
			char ipd[100];
			inet_ntop(AF_INET, &(remaddr.sin_addr), ipd, sizeof(ipd));
			
			ip = ipd;
			port = ntohs(remaddr.sin_port);

			return recvlen;
		}
	}
	
	return 0;
}
