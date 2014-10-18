#include "Misc_InternalPch.h"

#if defined(_WIN32)
#include <windows.h>
#pragma comment(lib, "winmm.lib")
#elif defined(__APPLE__)
#include <CoreServices/CoreServices.h>
#include <unistd.h>
#include <sys/time.h>
#endif // _WIN32

#include <stdio.h>
#include <assert.h>
#include "Map.h"
#include "AnsiString.h"
#include <time.h>

namespace Misc {

bool gInAssert = false;	

DWORD GetMilliseconds()
{
#if defined(_WIN32)
	return ::timeGetTime();
#endif
#if defined(PLATFORM_MAC)
	timeval time;
	gettimeofday(&time, NULL);
	return (time.tv_sec * 1000) + (time.tv_usec / 1000);
#endif // _WIN32
}


void SleepMilliseconds(unsigned int milliseconds)
{
#if defined(_WIN32)
	::Sleep(milliseconds);
#elif defined(PLATFORM_MAC)
	usleep(milliseconds * 1000);
#endif
}

#if defined(_WIN32)

bool CheckFor98Mill()
{
	static bool needOsCheck = true;
	static bool is98Mill = false;

	if (needOsCheck)
	{
		bool invalid = false;
		OSVERSIONINFOEXA osvi;
		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXA));

		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
		if( GetVersionExA((LPOSVERSIONINFOA)&osvi) == 0)
		{
			osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFOA);
			if ( GetVersionExA((LPOSVERSIONINFOA)&osvi) == 0)
				return false;
		}

		needOsCheck = false;
		is98Mill = osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS; // let's check Win95, 98, *AND* ME.
	}

	return is98Mill;
}

bool CheckForVista()
{
	static bool needOsCheck = true;
	static bool isVista = false;

	if (needOsCheck)
	{
		bool invalid = false;
		OSVERSIONINFOEXA osvi;
		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXA));

		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
		if( GetVersionExA((LPOSVERSIONINFOA)&osvi) == 0)
		{
			osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFOA);
			if ( GetVersionExA((LPOSVERSIONINFOA)&osvi) == 0)
				return false;
		}

		needOsCheck = false;
		isVista = osvi.dwMajorVersion >= 6;
	}

	return isVista;
}

bool CheckForTabletPC()
{
	static bool needsCheck = true;
	static bool isTabletPC = false;

	if (needsCheck)
	{
		isTabletPC = GetSystemMetrics(86) != 0; // check for tablet pc
		needsCheck = false;
	}

	return isTabletPC;
}


#endif // _WIN32

} // namespace Misc
