#include "../headers/const.hpp"
#include "../headers/ClipboardManager.h"
#include <comdef.h>
#include <codecvt>
/*
#include <gdiplusimaging.h>
#include <atlimage.h>
*/
#include <chrono>
#include <thread>

static int iResult = 0;
static int ignore_copy = 0;
static bool will_listen = true;
static HGLOBAL clipboard_value = nullptr;

ClipboardManager::ClipboardManager(HWND* window)
{
	_window = window;
	_config = get_config();
	_server_ip = _config.value("server_ip", "79.177.199.153").c_str();
}

ClipboardManager::~ClipboardManager()
{
	EndConnection();
    _devices.clear();
}

void ClipboardManager::EndConnection()
{
	will_listen = false;
	GlobalFree(clipboard_value);
	if (_sock != INVALID_SOCKET)
	{
		closesocket(_sock);
		WSACleanup();
	}
}

#pragma region Connecting to server 

void ClipboardManager::Begin()
{
	WSADATA wsaData;
	_sock = INVALID_SOCKET;
	struct addrinfo* result = NULL, *ptr = NULL, hints;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult)
		return;

	SecureZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(_server_ip, SERVER_PORT, &hints, &result);
	if (iResult != 0) 
	{
		WSACleanup();
		return;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) 
	{
		// Create a SOCKET for connecting to server
		_sock = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (_sock == INVALID_SOCKET) 
		{
			WSACleanup();
			return;
		}


		// Connect to server.
		iResult = connect(_sock, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) 
		{
			closesocket(_sock);
			_sock = INVALID_SOCKET;
			continue;
		}
		/*
		char* server_ip = inet_ntoa(((sockaddr_in*)ptr->ai_addr)->sin_addr);
		wchar_t* wserver_ip = new wchar_t[strlen(server_ip) + 1];
		mbtowc(wserver_ip, server_ip, strlen(server_ip) + 1);
		SetWindowText(*_window, wserver_ip);
		*/

		break;
	}

	freeaddrinfo(result);

	if (_sock == INVALID_SOCKET) 
	{
		WSACleanup();
		return;
	}

	int i = 1;
	setsockopt(_sock, IPPROTO_TCP, TCP_NODELAY, (char*)&i, sizeof(int)); //makes it so it doesn't merge packages

	//verification that the connection is okay
	char recvbuf[WELCOME_MESSAGE_LEN];

	iResult = recv(_sock, recvbuf, WELCOME_MESSAGE_LEN, 0);
	if (!iResult)
		EndConnection();
	std::thread lt(&ClipboardManager::ListenForUpdates, this);
	lt.detach();

}

#pragma endregion

#pragma region Updating Clipboard

void ClipboardManager::ListenForUpdates()
{
	char recvbuf[DEFAULT_BUFLEN];
	int delim_pos = 0, type = 0;
	using namespace std::this_thread;     // sleep_for, sleep_until
	using namespace std::chrono_literals; // ns, us, ms, s, h, etc.

	while (will_listen)
	{
		iResult = recv(_sock, recvbuf, DEFAULT_BUFLEN, 0);
		if (iResult > 0)
		{
			try
			{
				delim_pos = strlen(recvbuf);
				type = std::atoi(substr(recvbuf, 0, delim_pos).c_str());
				UpdateClipboard(substr(recvbuf, delim_pos + 2, iResult).c_str(), iResult - (delim_pos + 2), type);
			}
			catch (std::exception & e)
			{
				auto errmsg = e.what();
				int err = GetLastError();
			}
		}

		sleep_for(0.5s);
	}
}


void ClipboardManager::UpdateClipboard(const char* buff, int bufflen, int type)
{
	int buffsize = bufflen;
	if (type == CF_UNICODETEXT) //UTF16 nullbyte 0x00 0x00
		buffsize += 2;
	else if (type == CF_TEXT)//ANSI nullbyte 0x00
		buffsize++;

	clipboard_value = GlobalAlloc(GMEM_MOVEABLE, buffsize);

	if (clipboard_value)
	{
		auto clip = GlobalLock(clipboard_value);
		if (clip)
		{
			memset(clip, 0, bufflen);
			memcpy(clip, buff, bufflen);

			GlobalUnlock(clipboard_value);

			if (OpenClipboard(0))
			{
				ignore_copy = 2;

				EmptyClipboard();
				SetClipboardData(type, clipboard_value);

				CloseClipboard();
			}
		}
	}

}

#pragma endregion

#pragma region  Sending Clipboard

void ClipboardManager::SendClipboard(const char* bytes, int size, int type)
{
	//Send the clipboard format number
	std::string cf_type = std::to_string(type);
	int i = 0, message_size = cf_type.size() + DELIM_SIZE + size;

	char* msg = new char[message_size]{ 0 };
	strcpy_s(msg, cf_type.size() + 1, cf_type.c_str());

	for (i = 0; i < size; i++)
	{
		msg[cf_type.size() + DELIM_SIZE + i] = bytes[i];
	}


	// Send an initial buffer
	iResult = send(_sock, msg, message_size, 0);
	if (iResult == SOCKET_ERROR) 
	{
		EndConnection();
		return;
	}

	delete[] msg;
}

void ClipboardManager::SendText_Unicode(HANDLE& value)
{
	if (value)
	{
		const char16_t* utf16_text = (char16_t*)(GlobalLock(value));
		if (utf16_text)
		{
			std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conversion;
			std::string uft8_text = conversion.to_bytes(utf16_text);
			SendClipboard(uft8_text.c_str(), uft8_text.size(), CF_UNICODETEXT);
		}
	}
}

void ClipboardManager::SendText_ANSI(HANDLE& value)
{
	if (value)
	{
		const char* text = (char*)(GlobalLock(value));
		if (text)
			SendClipboard(text, strnlen(text, DEFAULT_BUFLEN), CF_TEXT);
	}
}

void ClipboardManager::SendStream(HBITMAP& value, int type)
{
	if (value && value != INVALID_HANDLE_VALUE)
	{
		void* dib = GlobalLock(value);
		if (dib)
		{
			/*
			DIB* info = reinterpret_cast<DIB*>(dib);
			BMP bmp = { 0 };
			bmp.header.type = 0x4D42;
			bmp.header.offset = 54;
			bmp.header.bfSize = info->biSizeImage + bmp.header.offset;
			bmp.dib = *info;

			std::string image = reinterpret_cast<char*>(&bmp.header.type);
			image = image + reinterpret_cast<char*>(&bmp.header.bfSize);
			image = image + reinterpret_cast<char*>(&bmp.header.reserved);
			image = image + reinterpret_cast<char*>(&bmp.header.offset);

			image = image + reinterpret_cast<char*>(&bmp.dib.biSize);
			image = image + reinterpret_cast<char*>(&bmp.dib.biWidth);
			image = image + reinterpret_cast<char*>(&bmp.dib.biHeight);
			image = image + reinterpret_cast<char*>(&bmp.dib.biPlanes);
			image = image + reinterpret_cast<char*>(&bmp.dib.biBitCount);
			image = image + reinterpret_cast<char*>(&bmp.dib.biCompression);
			image = image + reinterpret_cast<char*>(&bmp.dib.biSizeImage);
			image = image + reinterpret_cast<char*>(&bmp.dib.biXPelsPerMeter);
			image = image + reinterpret_cast<char*>(&bmp.dib.biYPelsPerMeter);
			image = image + reinterpret_cast<char*>(&bmp.dib.biClrUsed);
			image = image + reinterpret_cast<char*>(&bmp.dib.biClrImportant);
			image = image + reinterpret_cast<char*>(info + 1);

			SendClipboard("image", 5, type);*/
		}
	}
}

void ClipboardManager::AnalyzeClipboard(HWND hwnd)
{
	UINT type = 0;

	if (_sock != INVALID_SOCKET)
	{
		if (!ignore_copy)
		{
			if (OpenClipboard(hwnd))
			{
				type = EnumClipboardFormats(type); //gets the next available clipboard format. 0 if failed like u
				if (IsClipboardFormatAvailable(type)) //if the type is naisu
				{
					HANDLE value = nullptr;
					HBITMAP bmpValue = nullptr;
					switch (type)
					{
					case CF_BITMAP:
						bmpValue = (HBITMAP)GetClipboardData(type);
						SendStream(bmpValue, type);
						break;
					case CF_DIB:
						bmpValue = (HBITMAP)GetClipboardData(type);
						SendStream(bmpValue, type);
						break;
					case CF_DIBV5:
						bmpValue = (HBITMAP)GetClipboardData(type);
						SendStream(bmpValue, type);
						break;
					case CF_TEXT:
						value = GetClipboardData(type);
						SendText_ANSI(value);
						break;
					case CF_UNICODETEXT:
						value = GetClipboardData(type);
						SendText_Unicode(value);
						break;
					default:
						value = GetClipboardData(CF_TEXT);
						SendText_ANSI(value);
					}

					if(value)
						GlobalUnlock(value);
				}
				CloseClipboard();
			}
		}
		else
			ignore_copy--;
	}
}
#pragma endregion
