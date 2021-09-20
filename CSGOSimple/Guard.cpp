

	/*--------------------------\
	|    M0ne0N Guard System    |
	|      Coded: F1oth3r0      |
	|       VK: flotheryt       |
	\--------------------------*/

#include "features/lazy_importer.hpp"
//void silent_crash();



/*
string Guard::Auth(string login, string password)
{
	string authh = _xor_("/auth.php?login=");
	authh += login;
	authh += _xor_("&password=");
	authh += password;
	
	return GetUrlData(authh);
}*/

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

#define msgbox(a) MessageBoxA(0, a, "", 0)

VOID ErasePEHeaderFromMemory()
{
	DWORD OldProtect = 0;
	char* pBaseAddr = (char*)GetModuleHandle(NULL);
	VirtualProtect(pBaseAddr, 4096, PAGE_READWRITE, &OldProtect);
	SecureZeroMemory(pBaseAddr, 4096);
}

VOID SizeOfImage()
{
	PPEB pPeb = (PPEB)__readfsdword(0x30);
	PLDR_DATA_TABLE_ENTRY tableEntry = (PLDR_DATA_TABLE_ENTRY)(pPeb->Ldr->InMemoryOrderModuleList.Flink);
	tableEntry->DllBase = (PVOID)((INT_PTR)tableEntry->DllBase + 0x100000);
}

std::string GetHWID()
{
	HW_PROFILE_INFOA hwProfileInfo;
	std::string szHwProfileGuid = "";

	if (GetCurrentHwProfileA(&hwProfileInfo) != NULL)
		szHwProfileGuid = _xor_(hwProfileInfo.szHwProfileGuid);

	return szHwProfileGuid;
}

std::string GetCompUserName(bool User)
{
	std::string CompUserName = "";

	char szCompName[MAX_COMPUTERNAME_LENGTH + 1];
	char szUserName[MAX_COMPUTERNAME_LENGTH + 1];

	DWORD dwCompSize = sizeof(szCompName);
	DWORD dwUserSize = sizeof(szUserName);

	if (GetComputerNameA(szCompName, &dwCompSize))
		CompUserName = szCompName;

	if (User && GetUserNameA(szUserName, &dwUserSize))
		CompUserName = szUserName;

	return CompUserName;
}

std::string StringToHex(const std::string input)
{
	const char* lut = _xor_("0123456789ABCDEF").c_str();
	size_t len = input.length();
	std::string output = "";

	output.reserve(2 * len);

	for (size_t i = 0; i < len; i++)
	{
		const unsigned char c = input[i];
		output.push_back(lut[c >> 4]);
		output.push_back(lut[c & 15]);
	}

	return output;
}

std::string GetVolumeID()
{
	DWORD SerialNumber;

	if (GetVolumeInformationA(_xor_("C:\\").c_str(), NULL, NULL, &SerialNumber, NULL, NULL, NULL, NULL))
		return std::to_string(SerialNumber);

	return "";
}

std::string GetSerialKey()
{
	std::string key(StringToHex(GetHWID()));
	key.append(_xor_("-").c_str());
	key.append(StringToHex(GetCompUserName(true)));
	key.append(_xor_("-").c_str());
	key.append(StringToHex(GetCompUserName(false)));
	key.append(_xor_("-").c_str());
	key.append(StringToHex(GetVolumeID()));

	return key;
}

std::string GetHashText(const void* data, const size_t data_size)
{
	HCRYPTPROV hProv = NULL;

	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
		return "";

	BOOL hash_ok = FALSE;
	HCRYPTPROV hHash = NULL;

	hash_ok = CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash);

	if (!hash_ok)
	{
		CryptReleaseContext(hProv, 0);
		return "";
	}

	if (!CryptHashData(hHash, static_cast<const BYTE*>(data), data_size, 0))
	{
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		return "";
	}

	DWORD cbHashSize = 0, dwCount = sizeof(DWORD);
	if (!CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)& cbHashSize, &dwCount, 0))
	{
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		return "";
	}

	std::vector<BYTE> buffer(cbHashSize);

	if (!CryptGetHashParam(hHash, HP_HASHVAL, reinterpret_cast<BYTE*>(&buffer[0]), &cbHashSize, 0))
	{
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		return "";
	}

	std::ostringstream oss;

	for (std::vector<BYTE>::const_iterator iter = buffer.begin(); iter != buffer.end(); ++iter)
	{
		oss.fill('0');
		oss.width(2);
		oss << std::hex << static_cast<const int>(*iter);
	}

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);
	return oss.str();
}

std::string GetHashSerialKey()
{
	std::string SerialKey = GetSerialKey();
	for (auto& c : SerialKey)
	{
		if (c == '0') c = '2';
		else if (c == '1') c = '4';
		else if (c == '2') c = '6';
		else if (c == '3') c = '9';
		else if (c == '4') c = '0';
		else if (c == '5') c = '8';
		else if (c == '6') c = '3';
		else if (c == '7') c = '1';
		else if (c == '8') c = '5';
		else if (c == '9') c = '7';
		else if (c == 'a') c = 'd';
		else if (c == 'b') c = 'f';
		else if (c == 'c') c = 'a';
		else if (c == 'd') c = 'c';
		else if (c == 'e') c = 'f';
		else if (c == 'f') c = 'e';
		else if (c == '-') c = 'g';
		c = toupper(c);
	}
	return SerialKey;
}

std::string GetSerial()
{
	std::string Serial = GetHashSerialKey().substr(0, 13);
	Serial += GetHashSerialKey().substr(8, 17);
	Serial += GetHashSerialKey().substr(3, 15);
	Serial += GetHashSerialKey().substr(7, 10);
	return Serial;
}
static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
std::string base64_encode(char const* bytes_to_encode, unsigned int in_len)
{
	std::string ret;
	int i = 0, j = 0;
	unsigned char char_array_3[3], char_array_4[4];

	while (in_len--)
	{
		char_array_3[i++] = *(bytes_to_encode++);

		if (i == 3)
		{
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;
			for (i = 0; (i < 4); i++) ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++) char_array_3[j] = '\0';
		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		for (j = 0; (j < i + 1); j++) ret += base64_chars[char_array_4[j]];
		while ((i++ < 3)) ret += '=';
	}

	return ret;
}
std::string GetSerial64()
{
	return base64_encode(GetSerial().c_str(), GetSerial().size());
}
/*
std::string GetUrlData(std::string url)
{
	std::string request_data = "";

	HINTERNET hIntSession = InternetOpenA("", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!hIntSession) return request_data;

	HINTERNET hHttpSession = InternetConnectA(hIntSession, _xor_("api.m0ne0n.ru").c_str(), 80, 0, 0, INTERNET_SERVICE_HTTP, 0, NULL);
	if (!hHttpSession) return request_data;

	HINTERNET hHttpRequest = HttpOpenRequestA(hHttpSession, _xor_("GET").c_str(), url.c_str(), 0, 0, 0, INTERNET_FLAG_RELOAD, 0);
	if (!hHttpSession) return request_data;

	const char* szHeaders = _xor_("Content-Type: text/html\r\nUser-Agent: License").c_str();

	char szRequest[1024] = { 0 };

	if (!HttpSendRequestA(hHttpRequest, szHeaders, strlen(szHeaders), szRequest, strlen(szRequest)))
		return request_data;

	CHAR szBuffer[1024] = { 0 };
	DWORD dwRead = 0;

	while (InternetReadFile(hHttpRequest, szBuffer, sizeof(szBuffer) - 1, &dwRead) && dwRead)
		request_data.append(szBuffer, dwRead);

	InternetCloseHandle(hHttpRequest);
	InternetCloseHandle(hHttpSession);
	InternetCloseHandle(hIntSession);

	return request_data;
}

bool CheckVersion()
{
	std::string GetVER = GetUrlData(_xor_("/M0ne0N.php?ver").c_str());

	if (GetVER.size() && GetVER != "5")
		return true;
	return false;
}
bool Inject = false;
bool CheckLicense()
{
	std::string ReciveHash = GetUrlData(_xor_("/M0ne0N.php?hwid=").c_str() + GetSerial64());
	//MessageBoxA(0, ReciveHash.c_str(), "", 0);
	if (ReciveHash.size())
	{
		std::string abc_hashs[] = {
			_xor_("MzFBMEMzMjdDNTI0NEREMjlDQw=="),
			_xor_("RDNGOTVFRDcxMDI0RDA5M0Q4RjU="),
			_xor_("MTUyMjhFNkQ3OEMyOEEyQ0Q="),
			_xor_("MkJBRjhCOEMzNEUzRTVGMTI="),
			_xor_("MTlCOTEwRkVDMUE1NTE1MDA="),
			_xor_("MTI4NjU0M0QwRTRFMjk="),
			_xor_("MUI5OEQxNzIxOUUyOEQw"),
			_xor_("MTQ2RTA2RDI2NzgzQUFF")
		};

		for (int i = 0; i < sizeof(abc_hashs) / sizeof(*abc_hashs); ++i)
		{
			for (int RandomMD5 = 1323731; RandomMD5 <= 1323742; RandomMD5++)
			{
				std::string LicenseCheck = abc_hashs[i] + GetSerial64() + std::to_string(RandomMD5);
				if (ReciveHash == GetHashText(LicenseCheck.c_str(), LicenseCheck.size()))
				{
					//msgbox("OMG!");
					Inject = true;
					return true;
				}
			}
		}
	}

	Inject = false;
	return false;
}

std::string GetUsingHWID(std::string req)
{
	std::string Get = GetUrlData(_xor_("/M0ne0N.php?").c_str() + req + _xor_("=").c_str() + GetSerial64());

	if (Get.size())
		return Get;
	return "";
}
std::string GetNameViaSVR()
{
	try
	{
		static std::string GetUNAME = GetUrlData(_xor_("/M0ne0N.php?name=").c_str() + GetSerial64());;
		//if(!GetUNAME.size())
		//	GetUNAME 

		if (!GetUNAME.size())
			throw 81;

		return GetUNAME.c_str();
	}
	catch (int b) {
		silent_crash();
		return "";
	}
	return "";
}
*/