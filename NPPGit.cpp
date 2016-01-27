///----------------------------------------------------------------------------
/// Copyright (c) 2008-2010 
/// Brandon Cannaday
/// Paranoid Ferret Productions (support@paranoidferret.com)
///
/// This program is free software; you can redistribute it and/or
/// modify it under the terms of the GNU General Public License
/// as published by the Free Software Foundation; either
/// version 2 of the License, or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
///----------------------------------------------------------------------------

#include "stdafx.h"
#include "PluginInterface.h"
#include <string>
#include <vector>

const TCHAR PLUGIN_NAME[] = TEXT("Git");

static NppData nppData;
static std::vector<FuncItem> funcItems;

//
// Required plugin methods
//
extern "C" __declspec(dllexport) BOOL isUnicode()
{
	return TRUE;
}

extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
	nppData = notpadPlusData;
}

extern "C" __declspec(dllexport) const TCHAR * getName()
{
	return PLUGIN_NAME;
}

extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
	
}

extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
{
	*nbF = funcItems.size();
	return &funcItems.front();
}

///
/// Gets the path to the current file.
///
/// @return Current file path.
///
std::wstring getCurrentFile()
{
	TCHAR path[MAX_PATH];
	::SendMessage(nppData._nppHandle, NPPM_GETFULLCURRENTPATH, MAX_PATH, (LPARAM)path); 

	return std::wstring(path);
}

///
/// Gets the full path to every opened file.
///
/// @param numFiles [out] Returns the number of opened files.
///
/// @return Vector of filenames.
///
std::vector<std::wstring> getAllFiles()
{
   //get the number of opened files
   //notepad++ always returns an extra "new 1", remove it by subtracting 1
   int numFiles = (::SendMessage(nppData._nppHandle, NPPM_GETNBOPENFILES, 0, ALL_OPEN_FILES)) - 1;

   //allocate memory to hold filenames
   TCHAR** files = new TCHAR*[numFiles];
   for(int i = 0; i < numFiles; i++)
      files[i] = new TCHAR[MAX_PATH];

   //get filenames
   ::SendMessage(nppData._nppHandle, NPPM_GETOPENFILENAMES, (WPARAM)files, (LPARAM)numFiles);

   std::vector<std::wstring> filePaths;
   for(int i = 0; i < numFiles; i++)
      filePaths.push_back(files[i]);

   return filePaths;
}

///
/// Gets the path to the TortioseGit executable from the registry.
///
/// @param loc [out] Location of Tortoise executable
///
/// @return Whether or not the path was successfully retrieved.
///         If false, Tortoise is most likely not installed.
bool getTortoiseLocation(std::wstring &loc)
{
	HKEY hKey;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\TortoiseGit"), 0,
		KEY_READ | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
	{
		return false;
	}

	TCHAR procPath[MAX_PATH];
	DWORD length = MAX_PATH;

   // Modified Douglas Phillips <doug@sbscomp.com> 2008-12-29 to 
   // support 32-bit and Non-Vista operating Systems.
	if(RegQueryValueEx( 
         hKey, 
		   TEXT("ProcPath"), 
		   NULL, 
		   NULL, 
		   (LPBYTE)procPath, 
		   &length) != ERROR_SUCCESS)
	{
		return false;
	}

	loc = loc.append(procPath);
	return true;
}

///
/// Launches TortoiseGit using the supplied command
///
/// @param Command line string to execute.
///
/// @return Whether or not Tortoise could be launched.
///
bool launchTortoise(std::wstring &command)
{
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb = sizeof(si);

	return CreateProcess(
		NULL,
		const_cast<LPWSTR>(command.c_str()),
		NULL,
		NULL,
		FALSE,
		CREATE_DEFAULT_ERROR_MODE,
		NULL,
		NULL,
		&si,
		&pi) != 0;
}


///
/// Builds and executes command line string to send to CreateProcess
/// See http://tortoisesvn.net/docs/release/TortoiseSVN_en/tsvn-automation.html
/// for TortoiseSVN command line parameters.
///
/// @param cmd Command name to execute.
/// @param all Execute command on all files, or just the current file.
///
void ExecCommand(const std::wstring &cmd, bool all = false)
{
	std::wstring tortoiseLoc;
	bool tortoiseInstalled = getTortoiseLocation(tortoiseLoc);

	if(!tortoiseInstalled)
	{
		MessageBox(NULL, TEXT("Could not locate TortoiseGit"), TEXT("Update Failed"), 0);
		return;
	}

   std::vector<std::wstring> files;

   if (all)
      files = getAllFiles();
   else
      files.push_back(getCurrentFile());
   
	std::wstring command = tortoiseLoc;
	command += TEXT(" /command:") + cmd + TEXT(" /path:\"");

   for(std::vector<std::wstring>::iterator itr = files.begin();
      itr != files.end(); itr++)
   {
	   command += (*itr);
      if(itr != files.end() - 1)
         command += TEXT("*");
	   
   }
   command += TEXT("\" /closeonend:0");

	if(!launchTortoise(command))
		MessageBox(NULL, TEXT("Could not launch TortoiseGit."), TEXT("Update Failed"), 0);
}

void AddCommand(PFUNCPLUGINCMD func, LPCTSTR sMenuItemCaption,	UCHAR cShortCut)
{
	FuncItem item;

	item._pFunc = func;
	lstrcpy(item._itemName, sMenuItemCaption);
	item._init2Check = false;
	item._pShKey = new ShortcutKey;
	item._pShKey->_isAlt = true;
	item._pShKey->_isCtrl = true;
	item._pShKey->_isShift = false;
	item._pShKey->_key = cShortCut;

	funcItems.push_back(item);
}
////////////////////////////////////////////////////////////////////////////
///
/// Execution commands:
///
void commitFile()
{
   ExecCommand(TEXT("commit"));
}

void commitAllFiles()
{
   ExecCommand(TEXT("commit"), true);
}

void addFile()
{
   ExecCommand(TEXT("add"));
}

void diffFile()
{
   ExecCommand(TEXT("diff"));
}

void revertFile()
{
   ExecCommand(TEXT("revert"));
}

void revertAllFiles()
{
   ExecCommand(TEXT("revert"),true);
}

void showFileLog()
{
   ExecCommand(TEXT("log"));
}

void blameFile()
{
   ExecCommand(TEXT("blame"));
}

////////////////////////////////////////////////////////////////////////////
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			AddCommand(commitFile,		TEXT("Commit File"),			'C');
			AddCommand(commitAllFiles,	TEXT("Commit All Open Files"),	0);
			AddCommand(addFile,			TEXT("Add File"),				'A');
			AddCommand(diffFile,		TEXT("Diff File"),				'D');
			AddCommand(revertFile,		TEXT("Revert File"),			'R');
			AddCommand(revertAllFiles,	TEXT("Revert All Open Files"),	0);
			AddCommand(showFileLog,		TEXT("Show File Log"),			'L');
			AddCommand(blameFile,		TEXT("Blame File"),			0);
			break;

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

