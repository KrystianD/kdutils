#ifndef __DVR_H__
#define __DVR_H__

#include <stdint.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

class TDVRPacket;
class DVR
{
public:
	enum EError { OK = 0, ConnectError, ConnectionError, UnknownError, LoginError };
	DVR() : m_controlFd(-1) { }
	
	bool connectControl(const string& host, uint16_t port);
	void disconnect();
	
	bool login(const string& username, const string& pass);
	bool createChannel(int num, int frameCapacity);
	bool removeChannel(int channel);
	bool updateChannels();
	bool ping();
	
	bool checkForData();
	bool isAvail(int channel);
	bool grabFrame(int channel, char** buffer, int* len);
	bool releaseFrame(int channel);
	QVector<int> getChannels();
	
	const string& getLastErrorMsg() const
	{
		return m_lastErrorMsg;
	}
	
private:
	struct TStreamConnection
	{
		int fd;
		int channel;
		char *buffer;
		int bufferLen;
		
		int width, height, fps;


		// packet length (extension payload)
		int packetLen;
		// current read bytes
		int packetRead;
		// total frame length (built of multiple packets)
		int frameLen;
		int frameRead;
		bool frameReady;
	};
	
	string m_host;
	uint16_t m_port;
	int m_controlFd;
	uint32_t m_connId;
	int m_lastError;
	string m_lastErrorMsg;
	vector<TStreamConnection> m_conns;
	
	void setError(int err, const string& msg = "")
	{
		m_lastError = err;
		m_lastErrorMsg = msg;
	}
	
	int senddata(int fd, const uint8_t* data, int len);
	int readdata(int fd, uint8_t* data, int len);
};

#endif
