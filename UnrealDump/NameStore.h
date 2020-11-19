#pragma once
#include "pch.h"
#include "EngineClass.h"
#include "Offset.hpp"
class NameStore
{
	DWORD_PTR Names;
	DWORD NameSize;
public:
	static NameStore* GetInstance();
	static NameStore& GetReference();
	NameStore() ;
	virtual string operator[](int id);
	virtual string operator[](UObject* p);
	virtual string GetName(int id);
	virtual string GetName(UObject* id);
	virtual int GetNameSize();
};

