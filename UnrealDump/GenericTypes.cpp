#include "pch.h"
#include "GenericTypes.hpp"
#include "defs.h"
#include "NameStore.h"
#include "ObjectsStore.h"
#include "NameValidator.hpp"
#define GetOffsets(t, m) DWORD_PTR(&(((t*)0)->m))
//---------------------------------------------------------------------------
//UEObject
//---------------------------------------------------------------------------
size_t UEObject::GetIndex() const
{
	auto Index = Process::Read<decltype(UObject::Index)>((DWORD_PTR)object.GetPtr() + GetOffsets(UObject, Index));

	return 	Index;

}
//---------------------------------------------------------------------------
UEClass UEObject::GetClass() const
{

	auto Class = Process::Read<decltype(UObject::Class)>((DWORD_PTR)object.GetPtr() + GetOffsets(UObject, Class));
	return UEClass(Class);


}
//---------------------------------------------------------------------------
UEObject UEObject::GetOuter() const
{
	
	auto Outer =Process::Read<decltype(UObject::Outer)>((DWORD_PTR)object.GetPtr()+ GetOffsets(UObject, Outer));
	
	return UEObject(Outer);
}
//---------------------------------------------------------------------------
std::string UEObject::GetName() const
{
	auto index = Process::Read<decltype(UObject::ID)>((DWORD_PTR)object.GetPtr() + GetOffsets(UObject, ID));
	auto name = NameStore::GetReference().GetName(index);

	auto pos = name.rfind('/');
	if (pos == std::string::npos)
	{
		return name;
	}

	return name.substr(pos + 1);
}
//---------------------------------------------------------------------------
std::string UEObject::GetFullName() const
{
	if (GetClass().IsValid())
	{
		std::string temp;

		for (auto outer = GetOuter(); outer.IsValid(); outer = outer.GetOuter())
		{
			temp = outer.GetName() + "." + temp;
		}

		std::string name = GetClass().GetName();
		name += " ";
		name += temp;
		name += GetName();

		return name;
	}

	return std::string("(null)");
}
//---------------------------------------------------------------------------
void* UEObject::GetAddress() const
{
	return object.GetPtr();
}
//---------------------------------------------------------------------------
UEObject UEObject::GetPackageObject() const
{
	static auto PackageObject = [](){
		static auto Objects = ObjectsStore::GetReference();
		unordered_map<UEObject, UEObject> map;

		for (size_t i = 0; i < Objects.GetObjectsNum(); i++)
		{
			auto Obj = Objects[i];
			if (!Obj.IsValid()) continue;
			UEObject package(nullptr);
			for (auto outer = Obj.GetOuter(); outer.IsValid(); outer = outer.GetOuter())
			{
				package = outer;
			}
			map[Obj] = package;
		}
		return map;
	}();
	
	


	return PackageObject[*this];
}
//---------------------------------------------------------------------------
UEClass UEObject::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.Object");
	return c;
}
//---------------------------------------------------------------------------
std::string UEObject::GetNameCPP() const
{
	std::string name;

	if (IsA<UEClass>())
	{
		auto c = Cast<UEClass>();
		while (c.IsValid())
		{
			const auto className = c.GetName();
			if (className == "Actor")
			{
				name += "A";
				break;
			}
			if (className == "Object")
			{
				name += "U";
				break;
			}

			c = c.GetSuper().Cast<UEClass>();
		}
	}
	else
	{
		name += "F";
	}

	name += GetName();

	return name;
}
//---------------------------------------------------------------------------
//UEField
//---------------------------------------------------------------------------
UEField UEField::GetNext() const
{

	return UEField(Process::Read<decltype(UField::Next)>((DWORD_PTR)object.GetPtr() + GetOffsets(UField, Next)));
}
//---------------------------------------------------------------------------
UEClass UEField::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.Field");
	return c;
}
//---------------------------------------------------------------------------
//UEEnum
//---------------------------------------------------------------------------
std::vector<std::string> UEEnum::GetNames() const
{
	std::vector<std::string> buffer;
	auto names = Process::Read<decltype(UEnum::Names)>((DWORD_PTR)object.GetPtr() + GetOffsets(UEnum, Names));

	for (auto i = 0; i < names.Num(); ++i)
	{
		buffer.push_back(SimplifyEnumName(NameStore::GetReference()[names[i].Key.ComparisonIndex]));
	}

	return buffer;
}
//---------------------------------------------------------------------------

UEProperty::Info UEProperty::GetInfo() const
{
	if (IsValid())
	{
		if (IsA<UEByteProperty>())
		{
			return Cast<UEByteProperty>().GetInfo();
		}
		if (IsA<UEUInt16Property>())
		{
			return Cast<UEUInt16Property>().GetInfo();
		}
		if (IsA<UEUInt32Property>())
		{
			return Cast<UEUInt32Property>().GetInfo();
		}
		if (IsA<UEUInt64Property>())
		{
			return Cast<UEUInt64Property>().GetInfo();
		}
		if (IsA<UEInt8Property>())
		{
			return Cast<UEInt8Property>().GetInfo();
		}
		if (IsA<UEInt16Property>())
		{
			return Cast<UEInt16Property>().GetInfo();
		}
		if (IsA<UEIntProperty>())
		{
			return Cast<UEIntProperty>().GetInfo();
		}
		if (IsA<UEInt64Property>())
		{
			return Cast<UEInt64Property>().GetInfo();
		}
		if (IsA<UEFloatProperty>())
		{
			return Cast<UEFloatProperty>().GetInfo();
		}
		if (IsA<UEDoubleProperty>())
		{
			return Cast<UEDoubleProperty>().GetInfo();
		}
		if (IsA<UEBoolProperty>())
		{
			return Cast<UEBoolProperty>().GetInfo();
		}
		if (IsA<UEObjectProperty>())
		{
			return Cast<UEObjectProperty>().GetInfo();
		}
		if (IsA<UEEncryptedObjectProperty>())
		{
			return Cast<UEEncryptedObjectProperty>().GetInfo();
		}
		if (IsA<UEClassProperty>())
		{
			return Cast<UEClassProperty>().GetInfo();
		}
		if (IsA<UEInterfaceProperty>())
		{
			return Cast<UEInterfaceProperty>().GetInfo();
		}
		if (IsA<UEWeakObjectProperty>())
		{
			return Cast<UEWeakObjectProperty>().GetInfo();
		}
		if (IsA<UELazyObjectProperty>())
		{
			return Cast<UELazyObjectProperty>().GetInfo();
		}
		if (IsA<UEAssetObjectProperty>())
		{
			return Cast<UEAssetObjectProperty>().GetInfo();
		}
		if (IsA<UEAssetClassProperty>())
		{
			return Cast<UEAssetClassProperty>().GetInfo();
		}
		if (IsA<UENameProperty>())
		{
			return Cast<UENameProperty>().GetInfo();
		}
		if (IsA<UEStructProperty>())
		{
			return Cast<UEStructProperty>().GetInfo();
		}
		if (IsA<UEStrProperty>())
		{
			return Cast<UEStrProperty>().GetInfo();
		}
		if (IsA<UETextProperty>())
		{
			return Cast<UETextProperty>().GetInfo();
		}
		if (IsA<UEArrayProperty>())
		{
			return Cast<UEArrayProperty>().GetInfo();
		}
		if (IsA<UEMapProperty>())
		{
			return Cast<UEMapProperty>().GetInfo();
		}
		if (IsA<UEDelegateProperty>())
		{
			return Cast<UEDelegateProperty>().GetInfo();
		}
		if (IsA<UEMulticastDelegateProperty>())
		{
			return Cast<UEMulticastDelegateProperty>().GetInfo();
		}
		if (IsA<UEEnumProperty>())
		{
			return Cast<UEEnumProperty>().GetInfo();
		}
	}
	return { PropertyType::Unknown };
}

bool UEByteProperty::IsEnum() const
{
	return GetEnum().IsValid();
}




UEClass UEEnum::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.Enum");
	return c;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//UEStruct
//---------------------------------------------------------------------------
UEStruct UEStruct::GetSuper() const
{
	
	return UEStruct(Process::Read<decltype(UStruct::SueprField)>((DWORD_PTR)object.GetPtr() + GetOffsets(UStruct, SueprField)));
}
//---------------------------------------------------------------------------
UEField UEStruct::GetChildren() const
{

	return UEField(Process::Read<decltype(UStruct::Children)>((DWORD_PTR)object.GetPtr() + GetOffsets(UStruct, Children)));
}
//---------------------------------------------------------------------------
size_t UEStruct::GetPropertySize() const
{
	
	return Process::Read<decltype(UStruct::PropertySize)>((DWORD_PTR)object.GetPtr() + GetOffsets(UStruct, PropertySize));
}
//---------------------------------------------------------------------------
UEClass UEStruct::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.Struct");
	return c;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//UEScriptStruct
//---------------------------------------------------------------------------
UEClass UEScriptStruct::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.ScriptStruct");
	return c;
}
//---------------------------------------------------------------------------
//UEFunction
//---------------------------------------------------------------------------
UEFunctionFlags UEFunction::GetFunctionFlags() const
{
	
	return static_cast<UEFunctionFlags>(Process::Read<decltype(UFunction::FunctionFlags)>((DWORD_PTR)object.GetPtr() + GetOffsets(UFunction, FunctionFlags)));
}
//---------------------------------------------------------------------------
UEClass UEFunction::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.Function");
	return c;
}
void* UEFunction::GetFunctionAddress() const
{
	
	return Process::Read<decltype(UFunction::Fun)>((DWORD_PTR)object.GetPtr() + GetOffsets(UFunction, Fun));
}
//---------------------------------------------------------------------------
//UEClass
//---------------------------------------------------------------------------
UEClass UEClass::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.Class");
	return c;
}

//---------------------------------------------------------------------------
//UEProperty
//---------------------------------------------------------------------------
size_t UEProperty::GetArrayDim() const
{
	
	return  Process::Read<decltype(UProperty::ArrayDim)>((DWORD_PTR)object.GetPtr() + GetOffsets(UProperty, ArrayDim));
}
//---------------------------------------------------------------------------
size_t UEProperty::GetElementSize() const
{
	
	return Process::Read<decltype(UProperty::ElementSize)>((DWORD_PTR)object.GetPtr() + GetOffsets(UProperty, ElementSize));
}
//---------------------------------------------------------------------------
UEPropertyFlags UEProperty::GetPropertyFlags() const
{
	
	return static_cast<UEPropertyFlags>(Process::Read<decltype(UProperty::PropertyFlag)>((DWORD_PTR)object.GetPtr() + GetOffsets(UProperty, PropertyFlag)).A);
}
//---------------------------------------------------------------------------
size_t UEProperty::GetOffset() const
{
	
	return Process::Read<decltype(UProperty::offset)>((DWORD_PTR)object.GetPtr() + GetOffsets(UProperty, offset));
}
//---------------------------------------------------------------------------
UEClass UEProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.Property");
	return c;
}
//---------------------------------------------------------------------------
//UENumericProperty
//---------------------------------------------------------------------------
UEClass UENumericProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.NumericProperty");
	return c;
}
//---------------------------------------------------------------------------
//UEByteProperty
//---------------------------------------------------------------------------
UEEnum UEByteProperty::GetEnum() const
{
	
	return Process::Read<decltype(UEnumProperty::Enum)>((DWORD_PTR)object.GetPtr() + GetOffsets(UEnumProperty, Enum));
}
//---------------------------------------------------------------------------
UEProperty::Info UEByteProperty::GetInfo() const
{
	if (IsEnum())
	{
		return Info::Create(PropertyType::Primitive, sizeof(uint8_t), false, "TEnumAsByte<" + MakeUniqueCppName(GetEnum()) + ">");
	}
	return Info::Create(PropertyType::Primitive, sizeof(uint8_t), false, "unsigned char");
}
//---------------------------------------------------------------------------
UEClass UEByteProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.ByteProperty");
	return c;
}
//---------------------------------------------------------------------------
//UEUInt16Property
//---------------------------------------------------------------------------
UEProperty::Info UEUInt16Property::GetInfo() const
{
	return Info::Create(PropertyType::Primitive, sizeof(uint16_t), false, "uint16_t");
}
//---------------------------------------------------------------------------
UEClass UEUInt16Property::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.UInt16Property");
	return c;
}
//---------------------------------------------------------------------------
//UEUInt32Property
//---------------------------------------------------------------------------
UEProperty::Info UEUInt32Property::GetInfo() const
{
	return Info::Create(PropertyType::Primitive, sizeof(uint32_t), false, "uint32_t");
}
//---------------------------------------------------------------------------
UEClass UEUInt32Property::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.UInt32Property");
	return c;
}
//---------------------------------------------------------------------------
//UEUInt64Property
//---------------------------------------------------------------------------
UEProperty::Info UEUInt64Property::GetInfo() const
{
	return Info::Create(PropertyType::Primitive, sizeof(uint64_t), false, "uint64_t");
}
//---------------------------------------------------------------------------
UEClass UEUInt64Property::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.UInt64Property");
	return c;
}
//---------------------------------------------------------------------------
//UEInt8Property
//---------------------------------------------------------------------------
UEProperty::Info UEInt8Property::GetInfo() const
{
	return Info::Create(PropertyType::Primitive, sizeof(int8_t), false, "int8_t");
}
//---------------------------------------------------------------------------
UEClass UEInt8Property::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.Int8Property");
	return c;
}
//---------------------------------------------------------------------------
//UEInt16Property
//---------------------------------------------------------------------------
UEProperty::Info UEInt16Property::GetInfo() const
{
	return Info::Create(PropertyType::Primitive, sizeof(int16_t), false, "int16_t");
}
//---------------------------------------------------------------------------
UEClass UEInt16Property::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.Int16Property");
	return c;
}
//---------------------------------------------------------------------------
//UEIntProperty
//---------------------------------------------------------------------------
UEProperty::Info UEIntProperty::GetInfo() const
{
	return Info::Create(PropertyType::Primitive, sizeof(int), false, "int");
}
//---------------------------------------------------------------------------
UEClass UEIntProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.IntProperty");
	return c;
}
//---------------------------------------------------------------------------
//UEInt64Property
//---------------------------------------------------------------------------
UEProperty::Info UEInt64Property::GetInfo() const
{
	return Info::Create(PropertyType::Primitive, sizeof(int64_t), false, "int64_t");
}
//---------------------------------------------------------------------------
UEClass UEInt64Property::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.Int64Property");
	return c;
}
//---------------------------------------------------------------------------
//UEFloatProperty
//---------------------------------------------------------------------------
UEProperty::Info UEFloatProperty::GetInfo() const
{
	return Info::Create(PropertyType::Primitive, sizeof(float), false, "float");
}
//---------------------------------------------------------------------------
UEClass UEFloatProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.FloatProperty");
	return c;
}
//---------------------------------------------------------------------------
//UEDoubleProperty
//---------------------------------------------------------------------------
UEProperty::Info UEDoubleProperty::GetInfo() const
{
	return Info::Create(PropertyType::Primitive, sizeof(double), false, "double");
}
//---------------------------------------------------------------------------
UEClass UEDoubleProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.DoubleProperty");
	return c;
}
//---------------------------------------------------------------------------
//UEBoolProperty
//---------------------------------------------------------------------------
uint8_t UEBoolProperty::GetFieldSize() const
{
	
	return  Process::Read<decltype(UBoolProperty::FieldSize)>((DWORD_PTR)object.GetPtr() + GetOffsets(UBoolProperty, FieldSize));
}
//---------------------------------------------------------------------------
uint8_t UEBoolProperty::GetByteOffset() const
{
	
	
	return Process::Read<decltype(UBoolProperty::ByteOffset)>((DWORD_PTR)object.GetPtr() + GetOffsets(UBoolProperty, ByteOffset));
}
//---------------------------------------------------------------------------
uint8_t UEBoolProperty::GetByteMask() const
{
	
	return Process::Read<decltype(UBoolProperty::ByteMask)>((DWORD_PTR)object.GetPtr() + GetOffsets(UBoolProperty, ByteMask));
}
//---------------------------------------------------------------------------
uint8_t UEBoolProperty::GetFieldMask() const
{
	
	return  Process::Read<decltype(UBoolProperty::FieldMask)>((DWORD_PTR)object.GetPtr() + GetOffsets(UBoolProperty, FieldMask));
}
//---------------------------------------------------------------------------
UEProperty::Info UEBoolProperty::GetInfo() const
{
	if (IsNativeBool())
	{
		return Info::Create(PropertyType::Primitive, sizeof(bool), false, "bool");
	}
	return Info::Create(PropertyType::Primitive, sizeof(unsigned char), false, "unsigned char");
}
//---------------------------------------------------------------------------
UEClass UEBoolProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.BoolProperty");
	return c;
}
//---------------------------------------------------------------------------
//UEObjectPropertyBase
//---------------------------------------------------------------------------
UEClass UEObjectPropertyBase::GetPropertyClass() const
{
	
	return UEClass(Process::Read<decltype(UObjectPropertyBase::PropertyClass)>((DWORD_PTR)object.GetPtr() + GetOffsets(UObjectPropertyBase, PropertyClass)));
}
//---------------------------------------------------------------------------
UEClass UEObjectPropertyBase::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.ObjectPropertyBase");
	return c;
}
//---------------------------------------------------------------------------
//UEObjectProperty
//---------------------------------------------------------------------------
UEProperty::Info UEObjectProperty::GetInfo() const
{
	return Info::Create(PropertyType::Primitive, sizeof(void*), false, "class " + MakeValidName(GetPropertyClass().GetNameCPP()) + "*");
}
//---------------------------------------------------------------------------
UEClass UEObjectProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.ObjectProperty");
	return c;
}
//---------------------------------------------------------------------------
//UEClassProperty
//---------------------------------------------------------------------------
UEClass UEClassProperty::GetMetaClass() const
{

	
	
	return UEClass(Process::Read<decltype(UClassProperty::MetaClass)>((DWORD_PTR)object.GetPtr() + GetOffsets(UClassProperty, MetaClass)));
}
//---------------------------------------------------------------------------
UEProperty::Info UEClassProperty::GetInfo() const
{
	return Info::Create(PropertyType::Primitive, sizeof(void*), false, "class " + MakeValidName(GetMetaClass().GetNameCPP()) + "*");
}
//---------------------------------------------------------------------------
UEClass UEClassProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.ClassProperty");
	return c;
}
//---------------------------------------------------------------------------
//UEInterfaceProperty
//---------------------------------------------------------------------------
UEClass UEInterfaceProperty::GetInterfaceClass() const
{
	
	return UEClass(Process::Read<decltype(UInterfaceProperty::InterfaceClass)>((DWORD_PTR)object.GetPtr() + GetOffsets(UInterfaceProperty, InterfaceClass)));
}
//---------------------------------------------------------------------------
UEProperty::Info UEInterfaceProperty::GetInfo() const
{
	return Info::Create(PropertyType::PredefinedStruct, sizeof(FScriptInterface), true, "TScriptInterface<class " + MakeValidName(GetInterfaceClass().GetNameCPP()) + ">");
}
//---------------------------------------------------------------------------
UEClass UEInterfaceProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.InterfaceProperty");
	return c;
}
//---------------------------------------------------------------------------
//UEWeakObjectProperty
//---------------------------------------------------------------------------
UEProperty::Info UEWeakObjectProperty::GetInfo() const
{
	return Info::Create(PropertyType::Container, sizeof(FWeakObjectPtr), false, "TWeakObjectPtr<class " + MakeValidName(GetPropertyClass().GetNameCPP()) + ">");
}
//---------------------------------------------------------------------------
UEClass UEWeakObjectProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.WeakObjectProperty");
	return c;
}
//---------------------------------------------------------------------------
//UELazyObjectProperty
//---------------------------------------------------------------------------
UEProperty::Info UELazyObjectProperty::GetInfo() const
{
	return Info::Create(PropertyType::Container, sizeof(FLazyObjectPtr), false, "TLazyObjectPtr<class " + MakeValidName(GetPropertyClass().GetNameCPP()) + ">");
}
//---------------------------------------------------------------------------
UEClass UELazyObjectProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.LazyObjectProperty");
	return c;
}
//---------------------------------------------------------------------------
//UEAssetObjectProperty
//---------------------------------------------------------------------------
UEProperty::Info UEAssetObjectProperty::GetInfo() const
{
	return Info::Create(PropertyType::Container, sizeof(FAssetPtr), false, "TAssetPtr<class " + MakeValidName(GetPropertyClass().GetNameCPP()) + ">");
}
//---------------------------------------------------------------------------
UEClass UEAssetObjectProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.AssetObjectProperty");
	return c;
}
//---------------------------------------------------------------------------
//UEAssetClassProperty
//---------------------------------------------------------------------------
UEClass UEAssetClassProperty::GetMetaClass() const
{

	return UEClass(Process::Read<decltype(UAssetClassProperty::MetaClass)>((DWORD_PTR)object.GetPtr() + GetOffsets(UAssetClassProperty, MetaClass)));
}
//---------------------------------------------------------------------------
UEProperty::Info UEAssetClassProperty::GetInfo() const
{
	return Info::Create(PropertyType::Primitive, sizeof(uint8_t), false, "");
}
//---------------------------------------------------------------------------
UEClass UEAssetClassProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.AssetClassProperty");
	return c;
}
//---------------------------------------------------------------------------
//UENameProperty
//---------------------------------------------------------------------------
UEProperty::Info UENameProperty::GetInfo() const
{
	return Info::Create(PropertyType::PredefinedStruct, sizeof(FName), true, "struct FName");
}
//---------------------------------------------------------------------------
UEClass UENameProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.NameProperty");
	return c;
}
//---------------------------------------------------------------------------
//UEStructProperty
//---------------------------------------------------------------------------
UEScriptStruct UEStructProperty::GetStruct() const
{
	
	return UEScriptStruct(Process::Read<decltype(UStructProperty::Struct)>((DWORD_PTR)object.GetPtr() + GetOffsets(UStructProperty, Struct)));
}
//---------------------------------------------------------------------------
UEProperty::Info UEStructProperty::GetInfo() const
{
	return Info::Create(PropertyType::CustomStruct, GetElementSize(), true, "struct " + MakeUniqueCppName(GetStruct()));
}
//---------------------------------------------------------------------------
UEClass UEStructProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.StructProperty");
	return c;
}
//---------------------------------------------------------------------------
//UEStrProperty
//---------------------------------------------------------------------------
UEProperty::Info UEStrProperty::GetInfo() const
{
	return Info::Create(PropertyType::PredefinedStruct, sizeof(FString), true, "struct FString");
}
//---------------------------------------------------------------------------
UEClass UEStrProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.StrProperty");
	return c;
}
//---------------------------------------------------------------------------
//UETextProperty
//---------------------------------------------------------------------------
UEProperty::Info UETextProperty::GetInfo() const
{
	return Info::Create(PropertyType::PredefinedStruct, sizeof(FText), true, "struct FText");
}
//---------------------------------------------------------------------------
UEClass UETextProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.TextProperty");
	return c;
}
//---------------------------------------------------------------------------
//UEArrayProperty
//---------------------------------------------------------------------------
UEProperty UEArrayProperty::GetInner() const
{
	
	return UEProperty(Process::Read<decltype(UArrayProperty::Inner)>((DWORD_PTR)object.GetPtr() + GetOffsets(UArrayProperty, Inner)));
}
//---------------------------------------------------------------------------
UEProperty::Info UEArrayProperty::GetInfo() const
{
	const auto inner = GetInner().GetInfo();
	if (inner.Type != PropertyType::Unknown)
	{
		//extern IGenerator* generator;

		return Info::Create(PropertyType::Container, sizeof(TArray<void*>), false, "TArray<" + IGenerator::GetInstance()->GetOverrideType(inner.CppType) + ">");
	}

	return { PropertyType::Unknown };
}
//---------------------------------------------------------------------------
UEClass UEArrayProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.ArrayProperty");
	return c;
}
//---------------------------------------------------------------------------
//UEMapProperty
//---------------------------------------------------------------------------
UEProperty UEMapProperty::GetKeyProperty() const
{
	
	return UEProperty(Process::Read<decltype(UMapProperty::KeyProp)>((DWORD_PTR)object.GetPtr() + GetOffsets(UMapProperty, KeyProp)));
}
//---------------------------------------------------------------------------
UEProperty UEMapProperty::GetValueProperty() const
{
	
	return UEProperty(Process::Read<decltype(UMapProperty::ValueProp)>((DWORD_PTR)object.GetPtr() + GetOffsets(UMapProperty, ValueProp)));
}
//---------------------------------------------------------------------------
UEProperty::Info UEMapProperty::GetInfo() const
{
	const auto key = GetKeyProperty().GetInfo();
	const auto value = GetValueProperty().GetInfo();
	if (key.Type != PropertyType::Unknown && value.Type != PropertyType::Unknown)
	{
		//extern IGenerator* generator;

		return Info::Create(PropertyType::Container, 0x50, false, "TMap<" + IGenerator::GetInstance()->GetOverrideType(key.CppType) + ", " + IGenerator::GetInstance()->GetOverrideType(value.CppType) + ">");
	}

	return { PropertyType::Unknown };
}
//---------------------------------------------------------------------------
UEClass UEMapProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.MapProperty");
	return c;
}
//---------------------------------------------------------------------------
//UEDelegateProperty
//---------------------------------------------------------------------------
UEFunction UEDelegateProperty::GetSignatureFunction() const
{
	
	return UEFunction(Process::Read<decltype(UDelegateProperty::SignatureFunction)>((DWORD_PTR)object.GetPtr() + GetOffsets(UDelegateProperty, SignatureFunction)));
}
//---------------------------------------------------------------------------
UEProperty::Info UEDelegateProperty::GetInfo() const
{
	return Info::Create(PropertyType::PredefinedStruct, sizeof(FScriptDelegate), true, "struct FScriptDelegate");
}
//---------------------------------------------------------------------------
UEClass UEDelegateProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.DelegateProperty");
	return c;
}
//---------------------------------------------------------------------------
//UEMulticastDelegateProperty
//---------------------------------------------------------------------------
UEFunction UEMulticastDelegateProperty::GetSignatureFunction() const
{
	
	return UEFunction(Process::Read<decltype(UDelegateProperty::SignatureFunction)>((DWORD_PTR)object.GetPtr() + GetOffsets(UDelegateProperty, SignatureFunction)));
}
//---------------------------------------------------------------------------
UEProperty::Info UEMulticastDelegateProperty::GetInfo() const
{
	return Info::Create(PropertyType::PredefinedStruct, sizeof(FScriptMulticastDelegate), true, "struct FScriptMulticastDelegate");
}
//---------------------------------------------------------------------------
UEClass UEMulticastDelegateProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.MulticastDelegateProperty");
	return c;
}
//---------------------------------------------------------------------------
//UEEnumProperty
//---------------------------------------------------------------------------
UENumericProperty UEEnumProperty::GetUnderlyingProperty() const
{
	
	return UENumericProperty(Process::Read<decltype(UEnumProperty::UnderlyingProp)>((DWORD_PTR)object.GetPtr() + GetOffsets(UEnumProperty, UnderlyingProp)));
}
//---------------------------------------------------------------------------
UEEnum UEEnumProperty::GetEnum() const
{
	
	return UEEnum(Process::Read<decltype(UEnumProperty::Enum)>((DWORD_PTR)object.GetPtr() + GetOffsets(UEnumProperty, Enum)));
}
//---------------------------------------------------------------------------
UEProperty::Info UEEnumProperty::GetInfo() const
{
	return Info::Create(PropertyType::Primitive, sizeof(uint8_t), false, MakeUniqueCppName(GetEnum()));
}
//---------------------------------------------------------------------------
UEClass UEEnumProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.EnumProperty");
	return c;
}
//---------------------------------------------------------------------------






int GetBitPosition(uint8_t value)
{
	int i4 = !(value & 0xf) << 2;
	value >>= i4;

	int i2 = !(value & 0x3) << 1;
	value >>= i2;

	int i1 = !(value & 0x1);

	int i0 = (value >> i1) & 1 ? 0 : -8;

	return i4 + i2 + i1 + i0;
}

std::array<int, 2> UEBoolProperty::GetMissingBitsCount(const UEBoolProperty& other) const
{
	// If there is no previous bitfield member, just calculate the missing bits.
	if (!other.IsValid())
	{
		return { GetBitPosition(GetByteMask()), -1 };
	}

	// If both bitfield member belong to the same byte, calculate the bit position difference.
	if (GetOffset() == other.GetOffset())
	{
		return { GetBitPosition(GetByteMask()) - GetBitPosition(other.GetByteMask()) - 1, -1 };
	}

	// If they have different offsets, we need two distances
	// |00001000|00100000|
	// 1.   ^---^
	// 2.       ^--^

	return { std::numeric_limits<uint8_t>::digits - GetBitPosition(other.GetByteMask()) - 1, GetBitPosition(GetByteMask()) };
}

std::string UEConst::GetValue() const
{
	throw;
}

UEClass UEConst::StaticClass()
{
	//not supported by UE4
	return nullptr;
}

UEProperty::Info UEEncryptedObjectProperty::GetInfo() 
{
	return Info::Create(PropertyType::Primitive, sizeof(void*), false, "class " + MakeValidName(GetPropertyClass().GetNameCPP()) + "*");
}

UEClass UEEncryptedObjectProperty::StaticClass()
{
	static auto c = ObjectsStore::FindClass("Class CoreUObject.EncryptedObjectProperty");
	return c;
}

