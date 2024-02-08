#include <string>
#include <uchar.h>
#include <sstream>
#include <wchar.h>
#include <fstream>
#include <iostream>

#include "../headers/json.hpp"
// for convenience
using json = nlohmann::json;

#define CLIENT_PORT "6969"
#define SERVER_PORT "1337"
#define DEFAULT_BUFLEN 2048
#define MAX_CF_SIZE 3
#define DELIMETER "\0\0"
#define DELIM_SIZE 2
#define CONFIG_PATH ".\\config.json"

#define WELCOME_MESSAGE_LEN 5


#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <direct.h>

char * GetFullPath(const char* partialPath)
{
	char full[_MAX_PATH];
	_fullpath(full, partialPath, _MAX_PATH);
	return full;
}

json str_to_json(const char* jsonString) {
	json jsonObj;
	std::stringstream(jsonString) >> jsonObj;

	return jsonObj;
}

json get_config()
{
	std::string ctext, line;
	std::ifstream cfile(GetFullPath(CONFIG_PATH));
	if (cfile.is_open())
	{

		// Use a while loop together with the getline() function to read the file line by line
		while (std::getline(cfile, line)) {
			ctext = ctext + "\n" + line;
			std::cout << ctext;
		}

		// Close the file
		cfile.close();
	}
	else
	{
		perror(CONFIG_PATH);
		//std::cout << "Failed to open File" << std::endl;
	}

	return str_to_json(ctext.c_str());
}


typedef struct
{
	std::uint32_t biSize;
	std::int32_t  biWidth;
	std::int32_t  biHeight;
	std::uint16_t  biPlanes;
	std::uint16_t  biBitCount;
	std::uint32_t biCompression;
	std::uint32_t biSizeImage;
	std::int32_t  biXPelsPerMeter;
	std::int32_t  biYPelsPerMeter;
	std::uint32_t biClrUsed;
	std::uint32_t biClrImportant;
} DIB;

typedef struct
{
	std::uint16_t type;
	std::uint32_t bfSize;
	std::uint32_t reserved;
	std::uint32_t offset;
} HEADER;

typedef struct
{
	HEADER header;
	DIB dib;
} BMP;

static inline std::string U16toString(const std::u16string& wstr) {
	std::string str = "";
	char cstr[3] = "\0";
	mbstate_t mbs;
	for (const auto& it : wstr) {
		memset(&mbs, 0, sizeof(mbs));//set shift state to the initial state
		memmove(cstr, "\0\0\0", 3);
		c16rtomb(cstr, it, &mbs);
		str.append(std::string(cstr));
	}//for
	return str;
}

static inline std::u16string StringtoU16(const std::string& str) {
	std::u16string wstr = u"";
	char16_t c16str[3] = u"\0";
	mbstate_t mbs;
	for (const auto& it : str) {
		memset(&mbs, 0, sizeof(mbs));//set shift state to the initial state
		memmove(c16str, u"\0\0\0", 3);
		mbrtoc16(c16str, &it, 3, &mbs);
		wstr.append(std::u16string(c16str));
	}//for
	return wstr;
}

static inline std::string substr(const char* str, int from, int to)
{
	std::string res;
	for (int i = from; i < to; i++)
	{
		res += str[i];
	}
	return res;
}