#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <atlstr.h>

#include "../headers/json.hpp"
#include "../headers/Device.h"

// for convenience
using json = nlohmann::json;

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

class ClipboardManager
{
public:
	ClipboardManager(HWND*);
	~ClipboardManager();

	void Begin();
	void EndConnection();

	void AnalyzeClipboard(HWND);

	void SendClipboard(const char*, int, int);
	void SendText_Unicode(HANDLE&);
	void SendText_ANSI(HANDLE&);
	void SendStream(HBITMAP&, int);

	void UpdateClipboard(const char*, int, int);
	void ListenForUpdates();
private:
	std::vector<Device> _devices;
	SOCKET _sock = INVALID_SOCKET;
	HWND* _window;
	json _config;
	const char* _server_ip;
};