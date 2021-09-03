﻿// UnrealDump.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include "NameStore.h"
#include "ObjectsStore.h"
#include "Package.h"
#define Object ObjectsStore::GetReference()

//unordered_map<string, vector<string>> Offset;

void ProcessPackages()
{
	auto objlist =  []()->auto {
		vector<UEObject> objs;
		for (size_t i = 0; i < Object.GetObjectsNum(); i++)
		{
			auto obj = Object[i];
			if (obj.IsValid())
			{
				objs.push_back(obj);
			}
		}
		return objs; }();
		using namespace cpplinq;
		auto packageObjects = from(objlist)
		>> select([](auto&& o) { return o.GetPackageObject(); })
		>> where([](auto&& o) { return o.IsValid(); })
		>> distinct()
		>> to_vector();

	std::unordered_map<UEObject, bool> processedObjects;
	
	//json& GameObject = j["GameObject"];



	for (auto x : packageObjects)
	{
		cout << x.GetFullName() << endl;
	
		
		Package package(x);
		package.Process(objlist,processedObjects);
		package.Save();
		//package.SaveOffset(Offset);
		

	}

	MessageBox(0,"ok",0,0);

}


void Dump()
{
	FILE* NameDump = NULL;
	fopen_s(&NameDump, "NameDump.txt", "w+");	
	FILE* ObjectDump = NULL;
	fopen_s(&ObjectDump, "ObjectDump.txt", "w+");
	auto Names = NameStore::GetReference();
	for (size_t i = 0; i < Names.GetNameSize(); i++)
	{
		auto name = Names.GetName(i);
		if (!name.empty())
		{ 
			fprintf(NameDump, "Name[%06i] %s\n", i, name.c_str());
		}
	}
	fclose(NameDump);
	for (size_t i = 0; i < Object.GetObjectsNum(); i++)
	{
		auto Obj = Object[i];
		if (Obj.IsValid())
		{
			fprintf(ObjectDump, "UObject[%06i] %-100s 0x%p\n", i, Obj.GetFullName().c_str(),Obj.GetAddress());
		}
	}

	fclose(ObjectDump);
}
void OnStart()
{
	//auto Names = Process::XeDecryption(Process::Read<ULONG64>((DWORD_PTR)Process::GetBaseModule() + (DWORD64)GameInst::GNames));
	//auto NameSize = Process::XeDecryption(Process::Read<ULONG64>(Names + 0x8));
	

	//auto GlobalObjects = Process::XeDecryption(Process::Read < ULONG64>((ULONG64)Process::GetBaseModule() + (DWORD64)GameInst::GObject + 0x18) );
	//cout << hex << GlobalObjects << endl;
	//return;
	Dump();
	//cout << "Ok" << endl;
	//return;
	ProcessPackages();
}
int main()
{
	auto Pid = Process::GetProcessIdByName("Bless.exe").back();
	Process::Attach(Pid);
	thread(OnStart).join();
}