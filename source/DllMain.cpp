#include <Windows.h>
#include <string>
#include <filesystem>

extern "C"
{
	// huge dll proxying code ahead (except it doesn't proxy shit)
	// https://learn.microsoft.com/en-us/windows/win32/api/minidumpapiset/nf-minidumpapiset-minidumpreaddumpstream
	__declspec(dllexport) BOOL MiniDumpReadDumpStream(
		PVOID BaseOfDump,
		ULONG StreamNumber,
		void* Dir,
		void* StreamPointer,
		void* StreamSize
	)
	{
		return FALSE;
	}

	// https://learn.microsoft.com/en-us/windows/win32/api/minidumpapiset/nf-minidumpapiset-minidumpwritedump
	__declspec(dllexport) BOOL MiniDumpWriteDump(
		HANDLE hProcess, 
		DWORD ProcessId, 
		HANDLE hFile,
		DWORD Type, 
		void* ExceptionParam,
		void* UserStreamParam, 
		void* CallbackParam
	)
	{
		return FALSE;
	}
}

void load_our_dlls()
{
	// Query the image path - stolen from Aurie Core
	DWORD process_name_size = MAX_PATH;
	wchar_t process_name[MAX_PATH] = { 0 };
	if (!QueryFullProcessImageNameW(
		GetCurrentProcess(),
		0,
		process_name,
		&process_name_size
	))
	{
		return (void)MessageBoxA(
			nullptr,
			"Failed to query process path!",
			"Uniprox",
			MB_OK | MB_TOPMOST | MB_ICONERROR | MB_SETFOREGROUND
		);
	}

	const auto fspath = std::filesystem::path(process_name).parent_path() / "mods" / "native";
	printf("[uniprox] Searching directory: %S\n", fspath.native().c_str());

	std::error_code ec;
	for (const auto& entry : std::filesystem::directory_iterator(fspath, ec))
	{
		if (entry.is_regular_file(ec))
		{
			if (!entry.path().has_filename())
				continue;

			if (!entry.path().filename().has_extension())
				continue;

			if (!entry.path().filename().extension().compare(L".dll"))
			{
				printf("\t[uniprox] Loading %S\n", entry.path().filename().native().c_str());
				LoadLibraryW(entry.path().native().c_str());
			}
		}
	}

	printf("[uniprox] Finished searching directory.\n");
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason != DLL_PROCESS_ATTACH)
		return TRUE;

	if (AllocConsole())
	{
		FILE* dummy = nullptr;
		freopen_s(&dummy, "CONOUT$", "w", stdout);
	}
	
	printf("[uniprox] Spawning loader thread...\n");

	// TODO: Spawn another thread to do this, since loading shit from DllMain is forbidden by Microsoft.
	load_our_dlls();

	printf("[uniprox] Modules mapped successfully!\n");
	// Back up the console window
	HWND console_window = GetConsoleWindow();

	// Console can now be closed without killing the whole process
	FreeConsole();

	// Close the console
	PostMessage(console_window, WM_QUIT, 0, 0);

	return TRUE;
}
