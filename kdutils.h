#ifndef __KDUTILS_H__
#define __KDUTILS_H__

#include <stdint.h>
#include <string>
#include <vector>
#include <sys/types.h>
#include <string.h>

extern "C"
{
uint32_t getTicks();
uint32_t getTicksUS();
}
std::string getErrnoString();

#ifndef WIN32
int changeUidGid(int uid, gid_t gid);
#endif

std::vector<std::string> parseArgs(std::string str, int count);

#ifndef WIN32
char getch();
char getche();
#else
#include <conio.h>
#endif

#endif
