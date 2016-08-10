#include "kdstring.h"

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

vector<std::string> explode(const std::string& str, const std::string& delim, size_t maxCount, size_t start)
{
	vector<std::string> parts;
	size_t idx = start, delimIdx;
	
	delimIdx = str.find(delim, idx);
	if (delimIdx == string::npos)
	{
		parts.push_back(str);
		return parts;
	}
	do
	{
		if (parts.size() == maxCount - 1)
		{
			string part = str.substr(idx);
			parts.push_back(part);
			idx = str.size();
			break;
		}
		string part = str.substr(idx, delimIdx - idx);
		parts.push_back(part);
		idx = delimIdx + delim.size();
		delimIdx = str.find(delim, idx);
	}
	while (delimIdx != string::npos && idx < str.size());
	
	if (idx < str.size())
	{
		string part = str.substr(idx);
		parts.push_back(part);
	}
	
	return parts;
}
std::string replaceAll(const string& str, const string& what, const string& replacement)
{
	string newStr = "";
	size_t idx = 0;
	size_t pos;
	while (idx < str.size())
	{
		// printf ("idx: %d\n", idx);
		pos = str.find(what, idx);
		// printf ("pos: %d\n", pos);
		// printf ("p1os: %d\n", what.size ());
		if (pos == string::npos)
		{
			newStr += str.substr(idx);
			break;
		}
		else
		{
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
	string newStr = "";
	for (size_t i = 0; i < str.size(); i++)
	{
		if (chars.find_first_of(str[i]) == string::npos)
			newStr += str[i];
	}
	return newStr;
}
std::string joinStrings(const std::string& sep, const std::vector<std::string>& strings, size_t start, int count)
{
	string str = "";
	for (size_t i = start; i < strings.size() && count--; i++)
	{
		if (i != start)
			str += sep;
		str += strings[i];
	}
	return str;
}
std::string ltrim(const std::string& str, const std::string& chars)
{
	size_t idx = 0;
	for (; idx < str.size() && chars.find_first_of(str[idx]) != string::npos; idx++);
	return str.substr(idx);
}
std::string rtrim(const std::string& str, const std::string& chars)
{
	int idx = str.size() - 1;
	for (; idx >= 0 && chars.find_first_of(str[idx]) != string::npos; idx--);
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
