#include "kdutils.h"
#include "kdstring.h"

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

#if !(defined(WIN32) || defined(_WIN32))
#include <grp.h>
#endif

using namespace std;

uint32_t getTicks()
{
	timeval tv;
	gettimeofday(&tv, 0);
	uint32_t val = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	return val;
}
uint32_t getTicksUS()
{
	timeval tv;
	gettimeofday(&tv, 0);
	uint32_t val = tv.tv_sec * 1000000 + tv.tv_usec;
	return val;
}
std::string getErrnoString()
{
#if defined(WIN32) || defined(_WIN32)
	return "<noerr_win>";
#else
	char buf[256];
	char *str = strerror_r(errno, (char*)&buf, 256);
	return str;
#endif
}

#ifndef WIN32
int changeUidGid(int uid, gid_t gid)
{
	if (setgid(gid) == -1)
	{
		return 1;
	}
	gid_t gidl[1] = { gid };
	if (setgroups(1, gidl) == -1)
	{
		return 1;
	}
	if (setuid(uid) == -1)
	{
		return 1;
	}
	
	return 0;
}
#endif

std::vector<std::string> parseArgs(std::string str, int count)
{
	vector<std::string> args;
	str = trim(str);
	str = replaceAll(str, "\\\"", "\1");
	
	if (str.size() == 0) return args;
	
	if (count == 1)
	{
		args.push_back(str);
		return args;
	}
	
	vector<string> parts = explode(str, " ");
	vector<string> newParts;
	
	string tmpArg = "";
	bool capt = false;
	size_t i;
	for (i = 0; i < parts.size(); i++)
	{
		string p = parts[i];
		
		if (capt)
		{
			if (p.size() == 0)
			{
				tmpArg += " ";
			}
			else if (p[p.size() - 1] == '"')
			{
				tmpArg += " " + p.substr(0, p.size() - 1);
				newParts.push_back(tmpArg);
				tmpArg = "";
				capt = false;
			}
			else
			{
				tmpArg += " " + p;
			}
		}
		else
		{
			if (p.size() == 0)
			{
				// newParts.push_back (" ");
			}
			else if (p.size() > 1 && p[0] == '"' && p[p.size() - 1] == '"')
			{
				newParts.push_back(p.substr(1, p.size() - 2));
			}
			else if (p[0] == '"')
			{
				if (p.size() != 1)
					tmpArg = p.substr(1);
				else
					tmpArg = "";
				capt = true;
			}
			else
			{
				newParts.push_back(p);
			}
		}
		
		if (count != -1 && (int)newParts.size() == count - 1)
			break;
	}
	
	// for (int j = 0; j < parts.size (); j++)
	// {
	// cout << "p: |" << parts[j] << "|\n";
	// }
	// cout << endl;
	
	if (i + 1 < parts.size())
	{
		tmpArg = joinStrings(" ", parts, i + 1);
		newParts.push_back(tmpArg);
	}
	
	for (size_t j = 0; j < newParts.size(); j++)
	{
		newParts[j] = replaceAll(newParts[j], "\1", "\\\"");
		// cout << "np: |" << newParts[j] << "|\n";
	}
	
	// cout << "|"<<str<<"|\n";
	
	return newParts;
}

// getch
#ifndef WIN32
#include <termios.h>
#include <stdio.h>

static char _getch(int echo)
{
	struct termios oldCfg, newCfg;

	tcgetattr(0, &oldCfg);
	newCfg = oldCfg;
	newCfg.c_lflag &= ~ICANON;
	newCfg.c_lflag &= echo ? ECHO : ~ECHO;
	tcsetattr(0, TCSANOW, &newCfg);

	char ch = getchar();

	tcsetattr(0, TCSANOW, &oldCfg);
	return ch;
}
char getch()
{
	return _getch(0);
}
char getche()
{
	return _getch(1);
}
#endif
