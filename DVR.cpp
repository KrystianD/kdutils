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

#include <QString>
#include <QDebug>

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
		return *(uint32_t*)(raw + 4);
	}
	void setU32(int offset, uint32_t val)
	{
		*(uint32_t*)(raw + offset) = val;
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
		qDebug() << "DVR: closing channel: " << m_conns[i].channel;
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
	
	//pk.print ();
	
	m_connId = *(uint32_t*)(pk.raw + 16);
	
	//printf ("connId: 0x%08x\n", m_connId);
	/*
	
		TDVRPacket pk2;
		char* tt=
				"TransactionID:1\r\n"
				"Method:GetParameterNames\r\n"
				"ParameterName:Dahua.Device.VideoOut.General\r\n"
				"\r\n";
	
		pk2.setCmd (0xf4);
		pk2.setExtLen (strlen (tt));
	
		qDebug () << "1";
		res = senddata (m_controlFd, pk2.raw, 32);
		if (res == -1)
		{
			setError (ConnectionError, str (format ("unable1 to send login request packet: %1%") % strerror (errno)));
			return false;
		}
		qDebug () << "12";
		res = senddata (m_controlFd, (uint8_t*)tt, strlen (tt));
		if (res == -1)
		{
			setError (ConnectionError, str (format ("unable2 to send login request packet: %1%") % strerror (errno)));
			return false;
		}
	
		qDebug () << "123";
		res = readdata (m_controlFd, pk.raw, 32);
		if (res == -1)
		{
			setError (ConnectionError, str (format ("unable3 to read login response packet: %1%") % strerror (errno)));
			return false;
		}
		int len = pk.getExtLen ();
		qDebug () << len;
	
		char buf[2000];
		res = readdata (m_controlFd, (uint8_t*)buf, len);
		if (res == -1)
		{
			setError (ConnectionError, str (format ("unable3 to read login response packet: %1%") % strerror (errno)));
			return false;
		}
		//qDebug () << buf;
	
	*/
	return true;
}
bool DVR::createChannel(int num)
{
	TDVRPacket pk;
	int res;
	
	TStreamConnection strm;
	strm.channel = num;
	strm.buffer = 0;
	strm.bufferLen = 0;
	strm.packetLen = 0;
	strm.packetRead = 0;
	
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
	qDebug() << "created for fd" << strm.fd;
	
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
	qDebug() << "closed fd" << strm->fd;
	
	
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
		//if (ch != -1 && strm->channel != ch) continue;
		FD_SET(strm->fd, &fds);
		if (strm->fd > maxFd) maxFd = strm->fd;
	}
	
	timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0 * 1000;
	int res = select(maxFd + 1, &fds, 0, 0, &tv);
	if (res > 0)
	{
		for (size_t i = 0; i < m_conns.size(); i++)
		{
			TStreamConnection *strm = &m_conns[i];
			
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
				//qDebug () << QString (str (format ("%1%") % strm->fd).c_str ());
				
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
				
				if (strm->packetLen > strm->bufferLen)
				{
					if (strm->buffer)
						delete [] strm->buffer;
					strm->bufferLen = strm->packetLen * 150 / 100;
					strm->buffer = new char[strm->bufferLen];
				}
			}
			else if (strm->packetRead < strm->packetLen) // we're reading frame payload
			{
				int remaining = strm->packetLen - strm->packetRead;
				int r = read(strm->fd, strm->buffer + strm->packetRead, remaining);
				if (r <= 0)
				{
					setError(ConnectionError, getErrorText("unable to read frame data"));
					return false;
				}
				strm->packetRead += r;
			}
			else
			{
				qDebug() << "DO STH WITH THIS PACKET!!!!";
			}
		}
		return true;
	}
	else if (res == 0)
	{
		qDebug() << "noread";
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
	
	return strm->packetLen > 0 && strm->packetLen == strm->packetRead;
}
bool DVR::grabFrame(int channel, char** buffer, int* len)
{
	TDVRPacket pk;
	TStreamConnection *strm = 0;
	
	for (size_t i = 0; !strm && i < m_conns.size(); i++)
		if (m_conns[i].channel == channel)
			strm = &m_conns[i];
	if (!strm) return false;
	
	*buffer = strm->buffer;
	*len = strm->packetLen;
	strm->packetLen = 0;
	strm->packetRead = 0;
	
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
			qDebug() << "wait";
		}
		else if (res < 0)
		{
			return -1;
		}
	}
	return len;
}

