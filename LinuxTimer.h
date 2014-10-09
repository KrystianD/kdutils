#ifndef __TIMER_H__
#define __TIMER_H__

#include <iostream>
#include <sys/time.h>
#include <stdint.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <errno.h>

class LinuxTimer
{
public:
	LinuxTimer()
	{
		createTimer();
	}
	LinuxTimer(uint32_t secs)
	{
		createTimer();
		setInterval(secs);
	}
	LinuxTimer(uint32_t secs, uint32_t usecs)
	{
		createTimer();
		setInterval(secs, usecs);
	}
	~LinuxTimer()
	{
		close(m_timerfd);
	}
	
	void setInterval(uint32_t secs)
	{
		setInterval(secs, 0);
	}
	void setInterval(uint32_t secs, uint32_t usecs)
	{
		m_secs = secs;
		m_usecs = usecs;
		
		itimerspec spec;
		spec.it_interval.tv_sec = m_secs;
		spec.it_interval.tv_nsec = m_usecs * 1000;
		spec.it_value.tv_sec = m_secs;
		spec.it_value.tv_nsec = m_usecs * 1000;
		timerfd_settime(m_timerfd, 0, &spec, 0);
	}
	
	bool elapsed()
	{
		timeval tv;
		fd_set fds;
		
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		
		FD_ZERO(&fds);
		FD_SET(m_timerfd, &fds);
		
		int res = select(m_timerfd + 1, &fds, 0, 0, &tv);
		if (res == -1)
		{
			if (errno != EINTR)
			{
				return false;
			}
		}
		else
		{
			if (FD_ISSET(m_timerfd, &fds))
			{
				uint64_t val;
				int res = read(m_timerfd, &val, sizeof(uint64_t));
				return true;
			}
		}
		return false;
	}
	bool wait()
	{
		uint64_t val;
		int res = read(m_timerfd, &val, sizeof(uint64_t));
		return true;
	}
	
	int getFd() const
	{
		return m_timerfd;
	}
	
private:
	uint32_t m_secs, m_usecs;
	int m_timerfd;
	
	void createTimer()
	{
		m_timerfd = timerfd_create(CLOCK_REALTIME, 0);
	}
};

#endif
