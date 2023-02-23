#ifndef UNICODE_HPP
#define UNICODE_HPP

#include <string>

using namespace std;

u32string utf8ToUtf32(string utf8);
string utf32ToUtf8(u32string utf32);
u32string intToUtf32(int number);

#endif
