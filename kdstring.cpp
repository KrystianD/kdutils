#include "kdstring.h"

#include <stdio.h>
// #include <unistd.h>

std::vector<std::string> explode(const std::string& str, const std::string& delim, size_t maxCount, size_t start)
{
	std::vector<std::string> parts;
	size_t idx = start, delimIdx;

	delimIdx = str.find(delim, idx);
	if (delimIdx == std::string::npos) {
		parts.push_back(str);
		return parts;
	}
	do {
		if (parts.size() == maxCount - 1) {
			std::string part = str.substr(idx);
			parts.push_back(part);
			idx = str.size();
			break;
		}
		std::string part = str.substr(idx, delimIdx - idx);
		parts.push_back(part);
		idx = delimIdx + delim.size();
		delimIdx = str.find(delim, idx);
	} while (delimIdx != std::string::npos && idx < str.size());

	if (idx < str.size()) {
		std::string part = str.substr(idx);
		parts.push_back(part);
	}

	return parts;
}
std::string replaceAll(const std::string& str, const std::string& what, const std::string& replacement)
{
	std::string newStr = "";
	size_t idx = 0;
	size_t pos;
	while (idx < str.size()) {
		// printf ("idx: %d\n", idx);
		pos = str.find(what, idx);
		// printf ("pos: %d\n", pos);
		// printf ("p1os: %d\n", what.size ());
		if (pos == std::string::npos) {
			newStr += str.substr(idx);
			break;
		} else {
			// 012345
			// abcdcd
			newStr += str.substr(idx, pos - idx) + replacement;
			idx = pos + what.size();
		}
	}
	return newStr;
}
std::string removeChars(const std::string& str, const std::string& chars)
{
	std::string newStr = "";
	for (size_t i = 0; i < str.size(); i++) {
		if (chars.find_first_of(str[i]) == std::string::npos)
			newStr += str[i];
	}
	return newStr;
}
std::string joinStrings(const std::string& sep, const std::vector<std::string>& strings, size_t start, int count)
{
	std::string str = "";
	for (size_t i = start; i < strings.size() && count--; i++) {
		if (i != start)
			str += sep;
		str += strings[i];
	}
	return str;
}
std::string ltrim(const std::string& str, const std::string& chars)
{
	size_t idx = 0;
	for (; idx < str.size() && chars.find_first_of(str[idx]) != std::string::npos; idx++);
	return str.substr(idx);
}
std::string rtrim(const std::string& str, const std::string& chars)
{
	int idx = str.size() - 1;
	for (; idx >= 0 && chars.find_first_of(str[idx]) != std::string::npos; idx--);
	return str.substr(0, idx + 1);
}
std::string trim(const std::string& str, const std::string& chars)
{
	return ltrim(rtrim(str, chars), chars);
}
std::string xorStrings(const std::string& text, const std::string& key)
{
	std::string str = "";
	for (size_t i = 0; i < text.size(); i++)
		str += text[i] ^ key[i % key.size()];
	return str;
}

int toInt(const std::string& str)
{
	return atoi(str.c_str());
}
std::string toString(int val)
{
	char buf[20];
	sprintf(buf, "%d", val);
	return buf;
}
std::string toBinStr(uint8_t val)
{
	static char b[9];
	b[8] = 0;
	for (int i = 0; i < 8; i++)
	{
		b[7 - i] = (val & 0x01) | '0';
		val >>= 1;
	}
	return b;
}
void printBin(uint8_t val)
{
	printf("%s", toBinStr(val).c_str());
}
void printBinln(uint8_t val)
{
	printf("%s\n", toBinStr(val).c_str());
}
