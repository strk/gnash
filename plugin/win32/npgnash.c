// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include <windows.h>
#ifdef _DEBUG
#include <stdio.h>
#endif

HINSTANCE g_hInst;

int
main(int argc, const char *argv[])
{
	/* Satisfy SDL_main link issue. */
	return 0;
}

BOOL WINAPI
DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
#ifdef _DEBUG
    char szReason[64];

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            strcpy(szReason, "DLL_PROCESS_ATTACH");
            break;
        case DLL_THREAD_ATTACH:
            strcpy(szReason, "DLL_THREAD_ATTACH");
            break;
        case DLL_THREAD_DETACH:
            strcpy(szReason, "DLL_THREAD_DETACH");
            break;
        case DLL_PROCESS_DETACH:
            strcpy(szReason, "DLL_PROCESS_DETACH");
            break;
    }
    fprintf(stderr, "npgnash.dll, DllMain(): %s\n", szReason);
#endif

    g_hInst = hModule;
    return TRUE;
}
