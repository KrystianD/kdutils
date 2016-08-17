#ifndef __KDSTRING__
#define __KDSTRING__

#include <stdint.h>
#include <string>
#include <vector>
#include <sys/types.h>
#include <string.h>

std::vector<std::string> explode(const std::string& str, const std::string& delim, size_t maxCount = 0, size_t start = 0);
std::string replaceAll(const std::string& str, const std::string& what, const std::string& replacement);
std::string removeChars(const std::string& str, const std::string& chars);
std::string joinStrings(const std::string& sep, const std::vector<std::string>& strings, size_t start = 0, int count = -1);
std::string ltrim(const std::string& str, const std::string& chars = "\n\t\r ");
std::string rtrim(const std::string& str, const std::string& chars = "\n\t\r ");
std::string trim(const std::string& str, const std::string& chars = "\n\r\r ");
std::string xorStrings(const std::string& text, const std::string& key);

int toInt(const std::string& str);
std::string toString(int val);
std::string toBinStr(uint8_t val);
void printBin(uint8_t val);
void printBinln(uint8_t val);

#endif
