#include "pch.h"
#include "Process.hpp"
#include <TlHelp32.h>


 HANDLE Process::hProcess;
 DWORD Process::Pid;
vector<DWORD> Process::GetProcessIdByName(string name	)
{
	std::vector<DWORD> found;
	auto hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (!hProcSnap)
		return found;

	PROCESSENTRY32 tEntry = { 0 };
	tEntry.dwSize = sizeof(PROCESSENTRY32W);

	// Iterate threads
	for (BOOL success = Process32First(hProcSnap, &tEntry);
		success != FALSE;
		success = Process32Next(hProcSnap, &tEntry))
	{
		if (name.empty() || string(tEntry.szExeFile) == name.c_str())
			found.emplace_back(tEntry.th32ProcessID);
	}

	return found;
}
void EnableDebugPriv()
{
	HANDLE hToken;
	LUID sedebugnameValue;
	TOKEN_PRIVILEGES tkp;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))
	{
		return;
	}

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sedebugnameValue))
	{
		CloseHandle(hToken);
		return;
	}
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = sedebugnameValue;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof tkp, NULL, NULL))
	{
		CloseHandle(hToken);
	}
}
void Process::Attach(DWORD PID)
{
	EnableDebugPriv();
	Pid = PID;
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, PID);
}
HANDLE Process::GetBaseModule()
{
	MODULEENTRY32 moduleEntry;
	HANDLE handle = NULL;
	handle = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, Pid); //  获取进程快照中包含在th32ProcessID中指定的进程的所有的模块。
	if (!handle) {
		CloseHandle(handle);
		return NULL;
	}
	ZeroMemory(&moduleEntry, sizeof(MODULEENTRY32));
	moduleEntry.dwSize = sizeof(MODULEENTRY32);
	if (!Module32First(handle, &moduleEntry)) {
		CloseHandle(handle);
		return NULL;
	}

	CloseHandle(handle);
	return moduleEntry.hModule;
}

DWORD_PTR Process::XeDecryption(DWORD_PTR p)
{
	static auto DecFunction = reinterpret_cast<ULONG64(*)(ULONG64 key, ULONG64 base)>(0);
	try
	{
		static uint64_t Ptr = 0;
		if (DecFunction == nullptr)
		{
			int64_t DecryptPtr = 0;//Read<uint64_t>((DWORD_PTR)GetBaseModule() + (int)GameInst::XeDecryption);
			int32_t Tmp1Add = Read<uint32_t>(DecryptPtr + 3);
			Ptr = Tmp1Add + DecryptPtr + 7;
			unsigned char ShellcodeBuff[1024] = { NULL };
			ShellcodeBuff[0] = 0x90;
			ShellcodeBuff[1] = 0x90;
			ReadMemory((void*)DecryptPtr, &ShellcodeBuff[2], sizeof(ShellcodeBuff) - 2);//(void*)DecryptPtr
			ShellcodeBuff[2] = 0x48;
			ShellcodeBuff[3] = 0x8B;
			ShellcodeBuff[4] = 0xC1;
			ShellcodeBuff[5] = 0x90;
			ShellcodeBuff[6] = 0x90;
			ShellcodeBuff[7] = 0x90;
			ShellcodeBuff[8] = 0x90;
			DecFunction =reinterpret_cast<decltype(DecFunction)> (VirtualAlloc(nullptr, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));
			RtlCopyMemory((LPVOID)DecFunction, (LPVOID)ShellcodeBuff, sizeof(ShellcodeBuff));
		}

		return  DecFunction(Ptr, p);
	}
	catch (const std::exception&)
	{
		return NULL;
	}
}

bool Process::ReadMemory(PVOID address, PVOID buffer, size_t size)
{
	SIZE_T ret_size;
	return ReadProcessMemory(hProcess, address, buffer, size, &ret_size);
}

HANDLE Process::GetProcessModuleHandle(string ModuleName)
{
	MODULEENTRY32 moduleEntry;
	HANDLE handle = NULL;
	handle = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, Pid); //  获取进程快照中包含在th32ProcessID中指定的进程的所有的模块。
	if (!handle) {
		CloseHandle(handle);
		return NULL;
	}
	ZeroMemory(&moduleEntry, sizeof(MODULEENTRY32));
	moduleEntry.dwSize = sizeof(MODULEENTRY32);
	if (!Module32First(handle, &moduleEntry)) {
		CloseHandle(handle);
		return NULL;
	}
	do {
		if (string(moduleEntry.szModule) == ModuleName) {
			return moduleEntry.hModule;
		}
	} while (Module32Next(handle, &moduleEntry));
	CloseHandle(handle);
	return 0;
}