#define NOMINMAX
#include <Windows.h>
#include "config.hpp"
#include "valve_sdk/sdk.hpp"
#include "helpers/utils.hpp"
#include "helpers/input.hpp"
#include "xorstr.h"
#include "BASS/API.h"
#include "hooks.hpp"
#include "menu.hpp"
#include "options.hpp"
#include "render.hpp"
#include "security.hpp"
#include <chrono>
#include <thread>

void* saved_hwnd = nullptr;
//void silent_crash();
//bool CheckLicense();
auto CheckRemoteDebuggerPresentAPI()
{
	BOOL bIsDbgPresent = FALSE;
	CheckRemoteDebuggerPresent(GetCurrentProcess(), &bIsDbgPresent);
	return bIsDbgPresent;
}
void lockcursor()
{
	if (Menu::Get().IsVisible()) {
		g_VGuiSurface->UnlockCursor();
	}
	else {
		g_VGuiSurface->LockCursor();
	}
	g_Input->m_mouse_initiated = !Menu::Get().IsVisible();

	g_InputSystem->EnableInput(!Menu::Get().IsVisible());

	g_Input->m_mouse_active = Menu::Get().IsVisible();

	g_InputSystem->ResetInputState();
}

void swap_hwnd() {
	if (Menu::Get().IsVisible())
	{
		if (!saved_hwnd)
			swap(saved_hwnd, g_InputSystem->GetWindow());
	}
	else if (saved_hwnd)
		swap(saved_hwnd, g_InputSystem->GetWindow());
}
#include "features/lazy_importer.hpp"
#include "Guard.h"
void autoaccept();
//#include "VMP/VMProtectSDK.h"
DWORD WINAPI OnDllAttach(LPVOID base)
{
	while (!FindWindowA("Valve001", NULL))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	while (!GetModuleHandleA("serverbrowser.dll"))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	//while (true)
	{
	//	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	
#ifdef _DEBUG
	Utils::AttachConsole();
#endif

	try {
		Config->Setup();
		Utils::ConsolePrint(_xor_("Initializing...\n").c_str());
		Interfaces::Initialize();
		
#if PRIVATE == 2
		string GSRELJSJG3 = g_Guard.GetUrlData(_xor_("/check.php?add").c_str());
		if (!GSRELJSJG3.size())
			//LI_FN(silent_crash).get()();
#endif
		Interfaces::Dump();
#if PRIVATE != 2
		if (g_EngineClient->GetEngineBuildNumber() != 13739) {
			//silent_crash();
		}
#endif
		
		//CheckLicense();
		 

		InputSys::Get().Initialize();
		NetvarSys::Get().Initialize();
		Render::Get().Initialize();
	

		Menu::Get().Initialize();
		Hooks::Initialize();

		

		InputSys::Get().RegisterHotkey(VK_INSERT, [base]() {
			Menu::Get().Toggle();
			swap_hwnd();

		});
		//InputSys::Get().RegisterHotkey(VK_END, [base]()		{	g_Unload = true;		});

		Utils::ConsolePrint("Finished.\n");	
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		//FreeLibraryAndExitThread((HMODULE)base, 0);
		return false;
	}
	catch (const std::exception& ex) {
		Utils::ConsolePrint("An error occured during initialization:\n");
		Utils::ConsolePrint("%s\n", ex.what());
		Utils::ConsolePrint("Press any key to exit.\n");
		Utils::ConsoleReadKey();
		Utils::DetachConsole();

		FreeLibraryAndExitThread((HMODULE)base, 0);
	}
return false;
}

BOOL WINAPI OnDllDetach()
{
#ifdef _DEBUG
	Utils::DetachConsole();
#endif

	return TRUE;
}

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDll, _In_ DWORD fdwReason, _In_opt_ LPVOID lpvReserved)
{
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hinstDll);
		CreateThread(nullptr, NULL, OnDllAttach, hinstDll, NULL, nullptr);
		
		return TRUE;
	case DLL_PROCESS_DETACH:
		if (lpvReserved == nullptr)
			return OnDllDetach();
		return TRUE;
	default:
		return TRUE;
	}
}
