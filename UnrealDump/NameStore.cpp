#include "pch.h"
#include "NameStore.h"
#include "SmartPtr.hpp"
NameStore* NameStore::GetInstance()
{
    static auto _p = new NameStore();
    return _p;
}
NameStore& NameStore::GetReference()
{
    return *GetInstance();
}
NameStore::NameStore()
{
    Names = Process::XeDecryption(Process::Read<ULONG64>((DWORD_PTR)Process::GetBaseModule() + (int)GameInst::GNames));
    NameSize = Process::XeDecryption(Process::Read<ULONG64>(Names + 0x8));
    Names = Process::XeDecryption(Process::Read<ULONG64>(Names));
}
string NameStore::operator[](int id)
{
    return GetName(id);
}

string NameStore::operator[](UObject* p)
{
    return GetName(p);
}

string NameStore::GetName(int i)
{
    int Id = (int)(i  / (int)GameInst::ChunkSize);
    int Idtemp = (int)(i % (int)GameInst::ChunkSize);
    auto NamePtr = Process::Read<uint64_t>(Names + Id * 8);
    auto Name = Process::Read<uint64_t>(NamePtr + 8 * Idtemp);
    char name[0x100] = {0};
    if (Process::ReadMemory(PVOID(Name + 0x10), name, 0x100))
    {
        return name;
    }
    return string();
}

string NameStore::GetName(UObject* id)
{
    return string();
}

int NameStore::GetNameSize()
{
    return NameSize;
}
