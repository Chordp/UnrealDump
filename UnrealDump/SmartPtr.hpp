#pragma once

#include "Process.hpp"

template <class T>
class SmartPtr
{
	PVOID ptr;
public:
	SmartPtr() {};
	SmartPtr(PVOID p) :ptr(p) {};
	SmartPtr(DWORD_PTR p) :ptr((PVOID)p) {};
	PVOID GetPtr() const
	{
		return ptr;
	}
	bool IsValid() const
	{
		return ptr != nullptr;
	}
	operator T() const
	{
		
		return Process::Read<T>(ptr);
	}
	T operator*() const
	{
		return Process::Read<T>(ptr);
	}
	T operator[](size_t i) const
	{
		return Process::Read<T>((ULONG64)ptr + i * sizeof(T));
	}
	


};

