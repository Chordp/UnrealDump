#pragma once
#include "SmartPtr.hpp"
#include "EngineClass.h"
#include "GenericTypes.hpp"
class ObjectsStore
{
	ObjectsStore();
public:

	
	static ObjectsStore* GetInstance();
	static ObjectsStore& GetReference();
	size_t GetObjectsNum() const;
	SmartPtr<UObject>GetSmartById(size_t id) const;
	UEObject GetById(size_t id) const;
	UEObject operator[](size_t i) const;
	static UEClass FindClass(const std::string& name);

	template<class T>
	size_t CountObjects(const std::string& name) const
	{
		static std::unordered_map<std::string, size_t> cache;

		auto it = cache.find(name);
		if (it != std::end(cache))
		{
			return it->second;
		}

		size_t count = 0;
		for (size_t i = 0; i < GetObjectsNum(); i++)
		{
			auto obj = GetById(i);
			if (obj.IsA<T>() && obj.GetName() == name)
			{
				++count;
			}
		}
		cache[name] = count;

		return count;
	}
};

