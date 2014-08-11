/*
 * DVR.cpp
 * Copyright (C) 2014 Krystian Dużyński <krystian.duzynski@gmail.com>
 */

#include "DVR.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define LOG(x,...) fprintf(stderr, "[DVR] " x "\n", ##__VA_ARGS__)
#define LOGCH(ch, x,...) fprintf(stderr, "[DVR #%d] " x "\n", ch, ##__VA_ARGS__)

class TDVRPacket
{
public:
	unsigned char raw[32];
	
	TDVRPacket()
	{
		memset(raw, 0, sizeof(raw));
		//raw[3] = 0x28; // size_version
	}
	
	void setCmd(uint8_t cmd)
	{
		raw[0] = cmd;
	}
	uint8_t getCmd() const
	{
		return raw[0];
	}
	void setExtLen(uint32_t len)
	{
		setU32(4, len);
	}
	uint32_t getExtLen() const
	{
		return getU32(4);
	}
	void setU32(int offset, uint32_t val)
	{
		*(uint32_t*)(raw + offset) = val;
	}
	uint32_t getU32(int offset) const
	{
		return *(uint32_t*)(raw + offset);
	}
	void print()
	{
		for (int i = 0; i < 32; i++)
		{
			printf("0x%02x ", raw[i]);
		}
		printf("\n");
	}
};

std::string getErrorText(const std::string& txt)
{
	static char tmp[200];
	sprintf(tmp, "%s: %s", txt.c_str(), strerror(errno));
	return tmp;
}

bool DVR::connectControl(const string& host, uint16_t port)
{
	m_host = host;
	m_port = port;
	
	struct sockaddr_in serv_addr;
	m_controlFd = socket(AF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	
	hostent* ent = gethostbyname(m_host.c_str());
	if (!ent)
	{
		setError(ConnectError, "unable to resolve name");
		return false;
	}
	memcpy(&serv_addr.sin_addr, ent->h_addr_list[0], 4);
	
	if (connect(m_controlFd, (sockaddr*)&serv_addr, sizeof(serv_addr)))
	{
		setError(ConnectError, getErrorText("connect error"));
		return false;
	}
	
	return true;
}
void DVR::disconnect()
{
	for (size_t i = 0; i < m_conns.size(); i++)
	{
		LOG("closing channel %d", m_conns[i].channel);
		close(m_conns[i].fd);
		if (m_conns[i].buffer)
		{
			delete [] m_conns[i].buffer;
			m_conns[i].buffer = 0;
		}
	}
	m_conns.clear();
	if (m_controlFd != -1)
		close(m_controlFd);
	m_controlFd = -1;
}

bool DVR::login(const string& username, const string& pass)
{
	TDVRPacket pk;
	int res;
	
	char user_pass[20];
	sprintf(user_pass, "%s&&%s", username.c_str(), pass.c_str());
	
	strncpy((char*)&pk.raw[8], username.c_str(), 8);
	strncpy((char*)&pk.raw[16], pass.c_str(), 8);
	
	pk.setCmd(0xa0);
	//pk.raw[3] = 0x60;
	pk.raw[24] = 0x04;
	pk.raw[25] = 0x01;
	//pk.raw[30] = 0xa1;
	//pk.raw[31] = 0xaa;
	//pk.setExtLen (strlen (user_pass));
	
	res = senddata(m_controlFd, pk.raw, 32);
	if (res == -1)
	{
		setError(ConnectionError, getErrorText("unable to send login request packet"));
		return false;
	}	
	res = readdata(m_controlFd, pk.raw, 32);
	if (res == -1)
	{
		setError(ConnectionError, getErrorText("unable to read login response packet"));
		return false;
	}
	
	if (pk.getCmd() != 0xb0)
	{
		setError(ConnectionError, "invalid login response");
		return false;
	}
	
	res = pk.raw[8];
	if (res != 0)
	{
		int code = pk.raw[9];
		char buf[200];
		sprintf(buf, "channel request error - res: %d code: %d", res, code);
		setError(LoginError, buf);
		return false;
	}
	
	m_connId = *(uint32_t*)(pk.raw + 16);

	return true;
}
bool DVR::createChannel(int num, int frameCapacity)
{
	TDVRPacket pk;
	int res;
	
	TStreamConnection strm;
	strm.channel = num;
	strm.buffer = 0;
	strm.bufferLen = 0;
	strm.packetLen = 0;
	strm.packetRead = 0;
	strm.frameLen = 0;
	strm.frameRead = 0;
	strm.frameReady = 0;
	strm.buffer = new char[frameCapacity];
	
	struct sockaddr_in serv_addr;
	strm.fd = socket(AF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(m_port);
	
	hostent* ent = gethostbyname(m_host.c_str());
	memcpy(&serv_addr.sin_addr, ent->h_addr_list[0], 4);
	
	if (connect(strm.fd, (sockaddr*)&serv_addr, sizeof(serv_addr)))
	{
		setError(ConnectError, "Unable to create stream connection");
		return false;
	}
	
	pk.setCmd(0xf1);
	pk.setU32(8, m_connId);
	pk.raw[12] = 1;
	pk.raw[13] = num + 1;
	pk.raw[17] = 0; // 0-estabilish, 1-close
	
	res = senddata(strm.fd, pk.raw, 32);
	if (res == -1)
	{
		setError(ConnectionError, getErrorText("unable to send conn bind request packet"));
		return false;
	}
	res = readdata(strm.fd, pk.raw, 32);
	if (res == -1)
	{
		setError(ConnectionError, getErrorText("unable to read conn bind response packet"));
		return false;
	}
	
	res = pk.raw[14];
	if (res != 0)
	{
		int code = pk.raw[8];
		char buf[200];
		sprintf(buf, "channel request error - res: %d code: %d", res, code);
		setError(LoginError, buf);
		return false;
	}
	LOG("created for fd %d", strm.fd);
	
	m_conns.push_back(strm);
	
	return true;
}
bool DVR::removeChannel(int channel)
{
	TDVRPacket pk;
	int res;
	TStreamConnection *strm = 0;
	
	for (size_t i = 0; !strm && i < m_conns.size(); i++)
		if (m_conns[i].channel == channel)
			strm = &m_conns[i];
	if (!strm) return false;
	
	pk.setCmd(0xf1);
	pk.setU32(8, m_connId);
	pk.raw[12] = 1;
	pk.raw[13] = channel + 1;
	pk.raw[17] = 1; // 0-estabilish, 1-close
	
	res = senddata(strm->fd, pk.raw, 32);
	if (res == -1)
	{
		setError(ConnectionError, getErrorText("unable to send conn unbind request packet"));
		return false;
	}
	res = readdata(strm->fd, pk.raw, 32);
	if (res == -1)
	{
		setError(ConnectionError, getErrorText("unable to read conn unbind response packet"));
		return false;
	}
	
	res = pk.raw[14];
	if (res != 0)
	{
		int code = pk.raw[8];
		char buf[200];
		sprintf(buf, "channel request error - res: %d code: %d", res, code);
		setError(LoginError, buf);
		return false;
	}
	
	close(strm->fd);
	
	delete [] strm->buffer;
	for (size_t i = 0; i < m_conns.size(); i++)
		if (m_conns[i].channel == channel)
			m_conns.erase(m_conns.begin() + i);
			
	return true;
}
bool DVR::updateChannels()
{
	TDVRPacket pk;
	int res;
	
	pk.setCmd(0x11);
	pk.setExtLen(16);
	
	for (size_t i = 0; i < m_conns.size(); i++)
		pk.raw[8 + m_conns[i].channel] = 1; // 1-yes, 0-no
		
	res = senddata(m_controlFd, pk.raw, 32);
	if (res == -1)
	{
		setError(ConnectionError, getErrorText("unable to send channel start packet"));
		return false;
	}
	uint8_t extdata[16];
	memset(extdata, 0, 16);
	
	// if 1, only thumbnails are being sent
	for (size_t i = 0; i < m_conns.size(); i++)
		extdata[m_conns[i].channel] = 0;
		
	res = senddata(m_controlFd, extdata, 16);
	if (res == -1)
	{
		setError(ConnectionError, getErrorText("unable to send channel start extdata"));
		return false;
	}
	return true;
}
bool DVR::ping()
{
	TDVRPacket pk;
	int res;
	
	pk.setCmd(0xa1);
	
	res = senddata(m_controlFd, pk.raw, 32);
	if (res == -1)
	{
		setError(ConnectionError, getErrorText("unable to send ping packet"));
		return false;
	}
	/*qDebug () << "ping1";
	res = readdata (m_controlFd, pk.raw, 32);
	qDebug () << "ping2";
	if (res == -1)
	{
		setError (ConnectionError, str (format ("unable to read ping reply packet: %1%") % strerror (errno)));
		return false;
	}*/
	return true;
}
bool DVR::checkForData()
{
	fd_set fds;
	FD_ZERO(&fds);
	
	int maxFd = -1;
	for (size_t i = 0; i < m_conns.size(); i++)
	{
		TStreamConnection *strm = &m_conns[i];
		FD_SET(strm->fd, &fds);
		if (strm->fd > maxFd) maxFd = strm->fd;
	}
	
	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 200 * 1000;
	int res = select(maxFd + 1, &fds, 0, 0, &tv);
	if (res > 0)
	{
		for (size_t i = 0; i < m_conns.size(); i++)
		{
			TStreamConnection *strm = &m_conns[i];
			
			if (strm->frameReady)
				continue;

			if (!FD_ISSET(strm->fd, &fds))
				continue;
				
			if (strm->packetLen == 0) // we're waiting for header
			{
				TDVRPacket pk;
				res = readdata(strm->fd, pk.raw, 32);
				if (res == -1)
				{
					setError(ConnectionError, getErrorText("unable to read frame header"));
					return false;
				}
				
				if (pk.getCmd() != 0xbc)
				{
					char buf[200];
					sprintf(buf, "pk.getCmd () != 0xbc (%d)", (int)pk.getCmd());
					setError(UnknownError, buf);
					return false;
				}
				
				res = pk.raw[16];
				if (res != 0)
				{
					char buf[200];
					sprintf(buf, "invalid packet res %d", res);
					setError(ConnectionError, buf);
					return false;
				}
				
				strm->packetLen = pk.getExtLen();
				strm->packetRead = 0;
			}
			else if (strm->packetRead < strm->packetLen) // we're reading frame payload
			{
				int remaining = strm->packetLen - strm->packetRead;
				int r = read(strm->fd, strm->buffer + strm->frameRead, remaining);
				if (r <= 0)
				{
					setError(ConnectionError, getErrorText("unable to read frame data"));
					return false;
				}

				strm->packetRead += r;
				strm->frameRead += r;
				LOGCH(strm->channel, "packet %d", r);

				if (strm->frameLen == 0)
				{
					if (strm->packetRead >= 16)
					{
						strm->frameLen = *(uint32_t*)(strm->buffer + 12);
						LOGCH(strm->channel, "frameLen %d", strm->frameLen);
					}
				}

				if (strm->frameLen != 0)
				{
					if (strm->frameRead == strm->frameLen)
					{
						LOGCH(strm->channel, "got whole frame");
						strm->frameReady = 1;
					}
				}

				if (strm->packetRead == strm->packetLen)
				{
					LOGCH(strm->channel, "got whole packet");
					strm->packetRead = 0;
					strm->packetLen = 0;
				}
			}
		}
		return true;
	}
	else if (res == 0)
	{
		return true;
	}
	else if (res < 0)
	{
		setError(ConnectionError, getErrorText("select error"));
		return false;
	}
	return true;
}
bool DVR::isAvail(int channel)
{
	TStreamConnection *strm = 0;
	
	for (size_t i = 0; !strm && i < m_conns.size(); i++)
		if (m_conns[i].channel == channel)
			strm = &m_conns[i];
	if (!strm) return false;
	
	return strm->frameReady;
}
bool DVR::grabFrame(int channel, char** buffer, int* len)
{
	TStreamConnection *strm = 0;

	for (size_t i = 0; !strm && i < m_conns.size(); i++)
		if (m_conns[i].channel == channel)
			strm = &m_conns[i];
	if (!strm) return false;

	*buffer = strm->buffer;
	*len = strm->frameLen;

	return true;
}
bool DVR::releaseFrame(int channel)
{
	TStreamConnection *strm = 0;

	for (size_t i = 0; !strm && i < m_conns.size(); i++)
		if (m_conns[i].channel == channel)
			strm = &m_conns[i];
	if (!strm) return false;

	strm->frameLen = 0;
	strm->frameRead = 0;
	strm->frameReady = 0;
	LOGCH(channel, "frame released");

	return true;
}
QVector<int> DVR::getChannels()
{
	QVector<int> ch;
	for (size_t i = 0; i < m_conns.size(); i++)
		ch.append(m_conns[i].channel);
	return ch;
}

int DVR::senddata(int fd, const uint8_t* data, int len)
{
	int totalSent = 0;
	while (totalSent < len)
	{
		int remaining = len - totalSent;
		int wr = write(fd, data, remaining);
		if (wr <= 0)
			return -1;
		totalSent += wr;
		data += wr;
	}
	return len;
}
int DVR::readdata(int fd, uint8_t* data, int len)
{
	//int flags = fcntl (fd, F_GETFL, 0);
	///fcntl (fd, F_SETFL, flags | O_NONBLOCK);
	
	int totalRead = 0;
	while (totalRead < len)
	{
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 100 * 1000;
		int res = select(fd + 1, &fds, 0, 0, &tv);
		if (res == 1)
		{
			int remaining = len - totalRead;
			int r = read(fd, data, remaining);
			if (r <= 0)
				return -1;
			totalRead += r;
			data += r;
		}
		else if (res == 0)
		{
			LOG("wait");
		}
		else if (res < 0)
		{
			return -1;
		}
	}
	return len;
}

