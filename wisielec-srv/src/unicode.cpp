#include "unicode.hpp"

u32string utf8ToUtf32(string utf8){
    char32_t currentChar;
    u32string utf32 = U"";

    for(auto c = utf8.begin(); c < utf8.end(); c++){
        if((*c & 0x80) == 0){
            // Single-byte character
            currentChar = *c;
        }else if((*c & 0xe0) == 0xc0){
            // Two-byte character
            currentChar = (*c & 0x1f) << 6;
            c++;
            currentChar |= (*c & 0x3f);
        }else if((*c & 0xf0) == 0xe0){
            // Three-byte character
            currentChar = (*c & 0x0f) << 12;
            c++;
            currentChar |= (*c & 0x3f) << 6;
            c++;
            currentChar |= (*c & 0x3f);
        }else if((*c & 0xf8) == 0xf0){
            // Four-byte character
            currentChar = (*c & 0x03) << 18;
            c++;
            currentChar |= (*c & 0x3f) << 12;
            c++;
            currentChar |= (*c & 0x3f) << 6;
            c++;
            currentChar |= (*c & 0x3f);
        }else{
            // Invalid character
            return U"";
        }
        utf32 += currentChar;
    }

    return utf32;
}

string utf32ToUtf8(u32string utf32){
    string utf8 = "";

    for(auto c = utf32.begin(); c < utf32.end(); c++){
        if(*c <= 0x7f){
            // Single-byte character
            utf8 += *c;
        }else if(*c <= 0x7ff){
            // Two-byte character
            utf8 += 0xc0 | (*c >> 6);
            utf8 += 0x80 | (*c & 0x3f);
        }else if(*c <= 0xffff){
            // Three-byte character
            utf8 += 0xe0 | (*c >> 12);
            utf8 += 0x80 | ((*c >> 6) & 0x3f);
            utf8 += 0x80 | (*c & 0x3f);
        }else if(*c <= 0x10ffff){
            // Four-byte character
            utf8 += 0xf0 | (*c >> 18);
            utf8 += 0x80 | ((*c >> 12) & 0x3f);
            utf8 += 0x80 | ((*c >> 6) & 0x3f);
            utf8 += 0x80 | (*c & 0x3f);
        }else{
            // Invalid character
            return "";
        }
    }

    return utf8;
}

u32string intToUtf32(int number){
    return utf8ToUtf32(to_string(number));
}