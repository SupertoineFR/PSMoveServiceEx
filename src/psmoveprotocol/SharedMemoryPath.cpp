#include "SharedMemoryPath.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

std::wstring get_shared_memory_directory_wide()
{
	wchar_t appdata[MAX_PATH];
	const DWORD length = GetEnvironmentVariableW(L"APPDATA", appdata, MAX_PATH);

	if (length == 0 || length >= MAX_PATH)
	{
		// Fall back to the old behaviour (relative to the working directory).
		return L"shared_mem";
	}

	return std::wstring(appdata, length) + L"\\PSMoveService\\shared_mem";
}

std::string get_shared_memory_directory()
{
	char appdata[MAX_PATH];
	const DWORD length = GetEnvironmentVariableA("APPDATA", appdata, MAX_PATH);

	if (length == 0 || length >= MAX_PATH)
	{
		return "shared_mem";
	}

	return std::string(appdata, length) + "\\PSMoveService\\shared_mem";
}

// Called by boost::interprocess because BOOST_INTERPROCESS_SHARED_DIR_FUNC is defined.
namespace boost {
namespace interprocess {
namespace ipcdetail {
	void get_shared_dir(std::string &shared_dir)
	{
		shared_dir = get_shared_memory_directory();
	}

	void get_shared_dir(std::wstring &shared_dir)
	{
		shared_dir = get_shared_memory_directory_wide();
	}
} // namespace ipcdetail
} // namespace interprocess
} // namespace boost
#endif // _WIN32
