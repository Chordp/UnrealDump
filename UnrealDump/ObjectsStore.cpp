#include "pch.h"
#include "ObjectsStore.h"
class FUObjectItem
{
public:
	uint64_t N000001AE; //0x0000
	class UObject* Object; //0x0008
	uint64_t Flags; //0x0010
	uint64_t ClusterIndex; //0x0018
	uint64_t SerialNumber; //0x0020
	char pad_0028[8]; //0x0028
};

class TUObjectArray
{
public:
	char pad_0000[8]; //0x0000
	SmartPtr< class FUObjectItem> Objects; //0x0008
	uint64_t NumElements; //0x0010
};

class FUObjectArray
{
public:
	__int32 ObjFirstGCIndex; //0x0000
	__int32 ObjLastNonGCIndex; //0x0004
	__int32 MaxObjectsNotConsideredByGC; //0x0008
	__int32 OpenForDisregardForGC; //0x000C

	TUObjectArray ObjObjects; //0x0010

	TUObjectArray GetObjObjects()
	{



		TUObjectArray t;
		t = ObjObjects;
		
		t.Objects = (FUObjectItem*)Process::XeDecryption((ULONG64)ObjObjects.Objects.GetPtr());
		t.NumElements = Process::XeDecryption(ObjObjects.NumElements);
		return t;
	}
};
class FUObjectArray GlobalObjects;
ObjectsStore::ObjectsStore()
{
	GlobalObjects = Process::Read<FUObjectArray>(reinterpret_cast<DWORD64>(Process::GetBaseModule()) + (int)GameInst::GObject);
}

ObjectsStore* ObjectsStore::GetInstance()
{
	static auto _p = new ObjectsStore();
	return _p;
}

ObjectsStore& ObjectsStore::GetReference()
{
	return *GetInstance();
}

size_t ObjectsStore::GetObjectsNum()const
{
	static auto NumElements = GlobalObjects.GetObjObjects().NumElements;
	return NumElements;
}

SmartPtr<UObject> ObjectsStore::GetSmartById(size_t id) const
{
	return SmartPtr<UObject>(GetById(id).GetAddress());
}

UEObject ObjectsStore::GetById(size_t id) const
{
	static auto obj = GlobalObjects.GetObjObjects().Objects;
	return obj[id].Object;
}

UEObject ObjectsStore::operator[](size_t i) const
{
	return GetById(i);
}

UEClass ObjectsStore::FindClass(const std::string& name) 
{
	static auto f = []() ->auto
	{
		static auto Objects = ObjectsStore::GetReference();
		std::unordered_map<string, UEObject> map;
		for (size_t i = 0; i < Objects.GetObjectsNum(); i++)
		{
			auto obj = Objects[i];
			if (!obj.IsValid()) continue;
			map[obj.GetFullName()] = obj;
		}
		return map;
	}();
	if (f.count(name) != 0)
	{
		return f[name].Cast<UEClass>();
	}
	
	return UEClass(nullptr);
}

