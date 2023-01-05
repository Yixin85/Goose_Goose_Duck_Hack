#pragma once

#include <Windows.h>
#include<TlHelp32.h>
#include <iostream>
#include <tchar.h> // _tcscmp
#include <vector>

#include"utils.hpp"

Utils utils;

class Memory {
public:
	DWORD pID = NULL;
	HANDLE processHandle = NULL;
	int64_t gameAssemblyBaseAddress = NULL;

	Memory() {

		pID = get_porcId_by_name("Goose Goose Duck.exe");
		if (pID == NULL) {
			utils.print("Please Launch the game before running this debug tool!", "���ڴ򿪸���ǰ������Ϸ��");
			std::cout << std::endl;
			return;
		}
		utils.print("Detected game pid:", "��⵽��Ϸ����pid:");
		std::cout << pID << std::endl;

		processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
		if (processHandle == INVALID_HANDLE_VALUE || processHandle == NULL) { // error handling
			std::cout << "Failed to open process" << std::endl;
			return;
		}

		char gameAssemblyModuleName[] = "GameAssembly.dll";
		gameAssemblyBaseAddress = GetModuleBaseAddress(_T(gameAssemblyModuleName), pID);
	}

	template <typename var>
	bool write_mem(int64_t address, var value) {
		return WriteProcessMemory(processHandle, address, &value, sizeof(var), NULL);
	}

	template <typename var>
	var read_mem(int64_t address) {
		var value;
		ReadProcessMemory(processHandle, (LPCVOID)address, &value, sizeof(var), NULL);
		return value;
	}

	int64_t FindPointer(int64_t moduleBaseAddress, int offset_num, int64_t offsets[])
	{
		if (offset_num <= 0) {
			return NULL;
		}

		int64_t Address = moduleBaseAddress + offsets[0];
		Address = read_mem<int64_t>(Address);
		//ReadProcessMemory(processHandle, (LPCVOID)Address, &Address, sizeof(DWORD), NULL);

		if (Address == 0) {
			return NULL;
		}

		for (int i = 1; i < offset_num; i++) //Loop trough the offsets
		{
			Address = read_mem<int64_t>(Address);
			//ReadProcessMemory(processHandle, (LPCVOID)Address, &Address, sizeof(DWORD), NULL);
			if (Address == 0) {
				return NULL;
			}
			Address += offsets[i];
		}
		return Address;
	}

	int64_t FindPointer(int64_t moduleBaseAddress, std::vector<int64_t> offsets)
	{
		if (offsets.size() <= 0) {
			return NULL;
		}

		int64_t Address = moduleBaseAddress + offsets[0];
		Address = read_mem<int64_t>(Address);
		//ReadProcessMemory(processHandle, (LPCVOID)Address, &Address, sizeof(DWORD), NULL);

		if (offsets.size() <= 1) {
			return Address;
		}

		Address += offsets[1];

		for (int i = 2; i < offsets.size(); i++) //Loop trough the offsets
		{
			Address = read_mem<int64_t>(Address);
			//ReadProcessMemory(processHandle, (LPCVOID)Address, &Address, sizeof(DWORD), NULL);
			if (Address == 0) {
				return NULL;
			}
			Address += offsets[i];
		}
		return Address;
	}

private:
	/// <summary>
	/// ��ȡ64λ���̵�ģ��Ļ�ַ<para/>
	/// Get baseAddress of x64 process's module
	/// </summary>
	/// <param name="lpszModuleName">Name of module ģ�������</param>
	/// <param name="pID">pid of process ����pid</param>
	/// <returns>address ��ַ</returns>
	int64_t GetModuleBaseAddress(TCHAR* lpszModuleName, DWORD pID) {
		int64_t dwModuleBaseAddress = 0;
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pID); // make snapshot of all modules within process
		MODULEENTRY32 ModuleEntry32 = { 0 };
		ModuleEntry32.dwSize = sizeof(MODULEENTRY32);

		if (Module32First(hSnapshot, &ModuleEntry32)) //store first Module in ModuleEntry32
		{
			do {
				if (_tcscmp(ModuleEntry32.szModule, lpszModuleName) == 0) // if Found Module matches Module we look for -> done!
				{
					dwModuleBaseAddress = (int64_t)ModuleEntry32.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnapshot, &ModuleEntry32)); // go through Module entries in Snapshot and store in ModuleEntry32


		}
		CloseHandle(hSnapshot);
		return dwModuleBaseAddress;
	}

	/*
	//works in x86
	DWORD GetModuleBaseAddress(TCHAR* lpszModuleName, DWORD pID) {
		DWORD dwModuleBaseAddress = 0;
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pID); // make snapshot of all modules within process
		MODULEENTRY32 ModuleEntry32 = { 0 };
		ModuleEntry32.dwSize = sizeof(MODULEENTRY32);

		if (Module32First(hSnapshot, &ModuleEntry32)) //store first Module in ModuleEntry32
		{
			do {
				if (_tcscmp(ModuleEntry32.szModule, lpszModuleName) == 0) // if Found Module matches Module we look for -> done!
				{
					dwModuleBaseAddress = (DWORD)ModuleEntry32.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnapshot, &ModuleEntry32)); // go through Module entries in Snapshot and store in ModuleEntry32


		}
		CloseHandle(hSnapshot);
		return dwModuleBaseAddress;
	}
	*/

	/// <summary>
	/// ͨ�����ƻ�ȡ����pid<para/>
	/// Get process' pid by name
	/// </summary>
	/// <param name="targetProcess"></param>
	/// <returns></returns>
	static DWORD get_porcId_by_name(const std::string_view targetProcess) {
		DWORD procId = 0;
		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnap != INVALID_HANDLE_VALUE)
		{
			PROCESSENTRY32 procEntry;
			procEntry.dwSize = sizeof(procEntry);
			if (Process32First(hSnap, &procEntry))
			{
				do
				{
					if (!targetProcess.compare(procEntry.szExeFile))
					{
						procId = procEntry.th32ProcessID;
						//std::cout << "found pID:" << procId << std::endl;
						//break;
					}
				} while (Process32Next(hSnap, &procEntry));
			}
		}
		CloseHandle(hSnap);

		return procId;
	}
};