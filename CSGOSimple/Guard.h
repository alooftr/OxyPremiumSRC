
	/*--------------------------\
	|    M0ne0N Guard System    |
	|      Coded: F1oth3r0      |
	|       VK: flotheryt       |
	\--------------------------*/

#include <iostream>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <windows.h>
#include <WinUser.h>
#include <WinBase.h>
#include <Winternl.h>
#pragma comment(lib,"wininet.lib")
#include <WinInet.h>
#include "xorstr.h"

#include <iphlpapi.h> // form mac adress
#pragma comment(lib, "iphlpapi") // and it

using namespace std;

class Guard
{
private:
	string GetHWID();
	string GetCompUserName(bool user = true);
	string GetVolumeID();
	string GetMacID();
	string StringToHex(const string input);
	string GetSerialKey();
	string GetHashSerialKey();
	string GetSerial();

	

public:
//	string Auth(string login, string password);
	string GetNameViaSVR();
	bool CheckLicense();
	bool CheckVersion();
	string GetSerial64();
	string GetUrlData(string url);
	string GetHashText(const void* data, const size_t data_size);
	
};

inline Guard g_Guard;