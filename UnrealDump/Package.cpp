#include "pch.h"

#include "Package.h"
#include "ObjectsStore.h"
#include "NameValidator.hpp"
#include "Generator.hpp"
std::ofstream osStructs("shabi_structs.hpp");
std::ofstream osClass("shabi_classes.hpp");
std::ofstream osEnum("shabi_Enum.hpp");
std::ofstream osFunction("shabi_Function.hpp");
std::ofstream osAll("shabi_All.hpp");
std::ofstream osOffset("shabi_Offset.hpp");


Package::Package(const UEObject& _packageObj)
	: packageObj(_packageObj)
{

}
void Package::PrintConstant(std::ostream& os, const std::pair<std::string, std::string>& c) const
{
	tfm::format(os, "#define CONST_%-50s %s\n", c.first, c.second);
}
bool ComparePropertyLess(const UEProperty& lhs, const UEProperty& rhs)
{
	if (lhs.GetOffset() == rhs.GetOffset()
		&& lhs.IsA<UEBoolProperty>()
		&& rhs.IsA<UEBoolProperty>())
	{
		return lhs.Cast<UEBoolProperty>() < rhs.Cast<UEBoolProperty>();
	}

	return lhs.GetOffset() < rhs.GetOffset();
}
void Package::GenerateScriptStruct(const UEScriptStruct& scriptStructObj)
{
	//extern IGenerator* generator;

	ScriptStruct ss;
	ss.Name = scriptStructObj.GetName();
	ss.FullName = scriptStructObj.GetFullName();

	//Logger::Log("ScriptStruct: %-100s - instance: 0x%P", ss.Name, scriptStructObj.GetAddress());

	ss.NameCpp = MakeValidName(scriptStructObj.GetNameCPP());
	ss.NameCppFull = "struct ";

	ss.NameCppFull += MakeUniqueCppName(scriptStructObj);

	ss.Size = scriptStructObj.GetPropertySize();
	ss.InheritedSize = 0;

	size_t offset = 0;

	auto super = scriptStructObj.GetSuper();
	if (super.IsValid() && super != scriptStructObj)
	{
		ss.InheritedSize = offset = super.GetPropertySize();

		ss.NameCppFull += " : public " + MakeUniqueCppName(super.Cast<UEScriptStruct>());
	}

	std::vector<UEProperty> properties;
	for (auto prop = scriptStructObj.GetChildren().Cast<UEProperty>(); prop.IsValid(); prop = prop.GetNext().Cast<UEProperty>())
	{
		if (prop.GetElementSize() > 0
			&& !prop.IsA<UEScriptStruct>()
			&& !prop.IsA<UEFunction>()
			&& !prop.IsA<UEEnum>()
			&& !prop.IsA<UEConst>()
			)
		{
			properties.push_back(prop);
		}
	}
	std::sort(std::begin(properties), std::end(properties), ComparePropertyLess);

	GenerateMembers(scriptStructObj, offset, properties, ss.Members);

	scriptStructs.emplace_back(std::move(ss));
}
void Package::GeneratePrerequisites(const UEObject& obj, std::unordered_map<UEObject, bool>& processedObjects)
{
	if (!obj.IsValid())
	{
		return;
	}

	const auto isClass = obj.IsA<UEClass>();
	const auto isScriptStruct = obj.IsA<UEScriptStruct>();
	if (!isClass && !isScriptStruct)
	{
		return;
	}

	const auto name = obj.GetName();
	if (name.find("Default__") != std::string::npos
		|| name.find("<uninitialized>") != std::string::npos
		|| name.find("PLACEHOLDER-CLASS") != std::string::npos)
	{
		return;
	}

	processedObjects[obj] |= false;

	auto classPackage = obj.GetPackageObject();
	if (!classPackage.IsValid())
	{
		return;
	}

	if (classPackage != packageObj)
	{
		return;
	}

	if (processedObjects[obj] == false)
	{
		processedObjects[obj] = true;

		if (!isScriptStruct)
		{
			auto outer = obj.GetOuter();
			if (outer.IsValid() && outer != obj)
			{
				GeneratePrerequisites(outer, processedObjects);
			}
		}

		auto structObj = obj.Cast<UEStruct>();

		auto super = structObj.GetSuper();
		if (super.IsValid() && super != obj)
		{
			GeneratePrerequisites(super, processedObjects);
		}

		GenerateMemberPrerequisites(structObj.GetChildren().Cast<UEProperty>(), processedObjects);

		if (isClass)
		{
			GenerateClass(obj.Cast<UEClass>());
		}
		else
		{
			GenerateScriptStruct(obj.Cast<UEScriptStruct>());
		}
	}
}
void Package::Process(vector<UEObject>& packageObjects,std::unordered_map<UEObject, bool>& processedObjects)
{
	static auto object = ObjectsStore::GetReference();
	for (auto obj : packageObjects)
	{
		//auto obj = object[i];
		//if (!obj.IsValid())
		//{
		//	continue;
		//}
		const auto package = obj.GetPackageObject();
		if (packageObj == package)
		{
			if (obj.IsA<UEEnum>())
			{
				GenerateEnum(obj.Cast<UEEnum>());
			}
			else if (obj.IsA<UEClass>())
			{

				GeneratePrerequisites(obj, processedObjects);
			}
			else if (obj.IsA<UEScriptStruct>())
			{
				GeneratePrerequisites(obj, processedObjects);
			}
		}
		
	}
}
bool Package::Save()
{
	using namespace cpplinq;
	if ((from(enums) >> where([](auto&& e) { return !e.Values.empty(); }) >> any()
			|| from(scriptStructs) >> where([](auto&& s) { return !s.Members.empty(); }) >> any()
			|| from(classes) >> where([](auto&& c) {return !c.Members.empty() || !c.Methods.empty(); }) >> any()
			)
		)
	{
		SvaeEnum();
		SaveStructs();
		SaveClasses();
		




		return true;
	}

	//Logger::Log("skip empty Package: %s", packageObj.GetName());

	return false;
}
bool Package::SaveOffset(unordered_map<string, vector<string>>&j)
{
	using namespace cpplinq;
	if ((from(enums) >> where([](auto&& e) { return !e.Values.empty(); }) >> any()
		|| from(scriptStructs) >> where([](auto&& s) { return !s.Members.empty(); }) >> any()
		|| from(classes) >> where([](auto&& c) {return !c.Members.empty() || !c.Methods.empty(); }) >> any()))
	{
		for (auto c: classes)
		{
			if(j.count(c.NameCpp)!=0)
			{
				vector<string> variable = j[c.NameCpp];
				for (auto m : c.Members)
				{
					if (std::find(variable.begin(), variable.end(), m.Name) != variable.end())
					{
				
						osOffset << "// " << c.NameCpp << "\tType:"<< m.Type << endl
							<< m.Name << "\t=\t" << tfm::format("0x%X,//(0x%X)", m.Offset, m.Size) << endl << flush;
					}
				}
			}
		}
		if (!scriptStructs.empty())
		{
			for (auto&& s : scriptStructs) 
			{
				if (j.count(s.NameCpp) != 0)
				{
					vector<string> variable = j[s.NameCpp];
					for (auto m : s.Members)
					{ 
						if (std::find(variable.begin(), variable.end(), m.Name) != variable.end())
						{
							osOffset << "// Class:" << s.NameCpp << "\tType:" << m.Type << endl
								<< m.Name << "\t=\t" << tfm::format("0x%X,//(0x%X)", m.Offset, m.Size) << endl << flush;
						}
					}
				}
			}
		}
		return true;
	}
		return false;
}
void Package::GenerateEnum(const UEEnum& enumObj)
{
	Enum e;
	e.Name = MakeUniqueCppName(enumObj);
	if (e.Name.find("Default__") != std::string::npos
		|| e.Name.find("PLACEHOLDER-CLASS") != std::string::npos)
	{
		return;
	}
	e.FullName = enumObj.GetFullName();

	std::unordered_map<std::string, int> conflicts;
	for (auto&& s : enumObj.GetNames())
	{
		const auto clean = MakeValidName(std::move(s));

		const auto it = conflicts.find(clean);
		if (it == std::end(conflicts))
		{
			e.Values.push_back(clean);
			conflicts[clean] = 1;
		}
		else
		{
			e.Values.push_back(clean + tfm::format("%02d", it->second));
			conflicts[clean]++;
		}
	}
	enums.emplace_back(std::move(e));

}
void Package::GenerateMembers(const UEStruct& structObj, size_t offset, const std::vector<UEProperty>& properties, std::vector<Member>& members) const
{
	//extern IGenerator* generator;

	std::unordered_map<std::string, size_t> uniqueMemberNames;
	size_t unknownDataCounter = 0;
	UEBoolProperty previousBitfieldProperty;

	for (auto&& prop : properties)
	{
		if (offset < prop.GetOffset())
		{
			previousBitfieldProperty = UEBoolProperty();

			const auto size = prop.GetOffset() - offset;
			members.emplace_back(CreatePadding(unknownDataCounter++, offset, size, "MISSED OFFSET"));
		}

		const auto info = prop.GetInfo();
		if (info.Type != UEProperty::PropertyType::Unknown)
		{
			Member sp;
			sp.Offset = prop.GetOffset();
			sp.Size = info.Size;

			sp.Type = info.CppType;
			sp.Name = MakeValidName(prop.GetName());

			const auto it = uniqueMemberNames.find(sp.Name);
			if (it == std::end(uniqueMemberNames))
			{
				uniqueMemberNames[sp.Name] = 1;
			}
			else
			{
				++uniqueMemberNames[sp.Name];
				sp.Name += tfm::format("%02d", it->second);
			}

			if (prop.GetArrayDim() > 1)
			{
				sp.Name += tfm::format("[0x%X]", prop.GetArrayDim());
			}

			if (prop.IsA<UEBoolProperty>() && prop.Cast<UEBoolProperty>().IsBitfield())
			{
				auto boolProp = prop.Cast<UEBoolProperty>();

				const auto missingBits = boolProp.GetMissingBitsCount(previousBitfieldProperty);
				if (missingBits[1] != -1)
				{
					if (missingBits[0] > 0)
					{
						members.emplace_back(CreateBitfieldPadding(unknownDataCounter++, previousBitfieldProperty.GetOffset(), info.CppType, missingBits[0]));
					}
					if (missingBits[1] > 0)
					{
						members.emplace_back(CreateBitfieldPadding(unknownDataCounter++, sp.Offset, info.CppType, missingBits[1]));
					}
				}
				else if (missingBits[0] > 0)
				{
					members.emplace_back(CreateBitfieldPadding(unknownDataCounter++, sp.Offset, info.CppType, missingBits[0]));
				}

				previousBitfieldProperty = boolProp;

				sp.Name += " : 1";
			}
			else
			{
				previousBitfieldProperty = UEBoolProperty();
			}

			sp.Flags = static_cast<size_t>(prop.GetPropertyFlags());
			sp.FlagsString = StringifyFlags(prop.GetPropertyFlags());

			members.emplace_back(std::move(sp));

			const auto sizeMismatch = static_cast<int>(prop.GetElementSize() * prop.GetArrayDim()) - static_cast<int>(info.Size * prop.GetArrayDim());
			if (sizeMismatch > 0)
			{
				members.emplace_back(CreatePadding(unknownDataCounter++, offset, sizeMismatch, "FIX WRONG TYPE SIZE OF PREVIOUS PROPERTY"));
			}
		}
		else
		{
			const auto size = prop.GetElementSize() * prop.GetArrayDim();
			members.emplace_back(CreatePadding(unknownDataCounter++, offset, size, "UNKNOWN PROPERTY: " + prop.GetFullName()));
		}

		offset = prop.GetOffset() + prop.GetElementSize() * prop.GetArrayDim();
	}

	if (offset < structObj.GetPropertySize())
	{
		const auto size = structObj.GetPropertySize() - offset;
		members.emplace_back(CreatePadding(unknownDataCounter++, offset, size, "MISSED OFFSET"));
	}
}
void Package::GenerateClass(const UEClass& classObj)
{
	//extern IGenerator* generator;

	Class c;
	c.Name = classObj.GetName();
	c.FullName = classObj.GetFullName();

	c.NameCpp = MakeValidName(classObj.GetNameCPP());
	c.NameCppFull = "class " + c.NameCpp;

	c.Size = classObj.GetPropertySize();
	c.InheritedSize = 0;

	size_t offset = 0;

	auto super = classObj.GetSuper();
	if (super.IsValid() && super != classObj)
	{
		c.InheritedSize = offset = super.GetPropertySize();

		c.NameCppFull += " : public " + MakeValidName(super.GetNameCPP());
	}
	
	{
		std::vector<UEProperty> properties;
		for (auto prop = classObj.GetChildren().Cast<UEProperty>(); prop.IsValid(); prop = prop.GetNext().Cast<UEProperty>())
		{
			if (prop.GetElementSize() > 0
				&& !prop.IsA<UEScriptStruct>()
				&& !prop.IsA<UEFunction>()
				&& !prop.IsA<UEEnum>()
				&& !prop.IsA<UEConst>()
				&& (!super.IsValid()
					|| (super != classObj
						&& prop.GetOffset() >= super.GetPropertySize()
						)
					)
				)
			{
				properties.push_back(prop);
			}
		}
		std::sort(std::begin(properties), std::end(properties), ComparePropertyLess);

		GenerateMembers(classObj, offset, properties, c.Members);
	}


	GenerateMethods(classObj, c.Methods);



	classes.emplace_back(std::move(c));
}
void Package::GenerateMemberPrerequisites(const UEProperty& first, std::unordered_map<UEObject, bool>& processedObjects)
{
	using namespace cpplinq;

	for (auto prop = first; prop.IsValid(); prop = prop.GetNext().Cast<UEProperty>())
	{
		const auto info = prop.GetInfo();
		auto x = prop.GetName();
		if (info.Type == UEProperty::PropertyType::Primitive)
		{
			
			if (prop.IsA<UEByteProperty>())
			{
				auto byteProperty = prop.Cast<UEByteProperty>();
				if (byteProperty.IsEnum())
				{
					//AddDependency(byteProperty.GetEnum().GetPackageObject());
				}
			}
			else if (prop.IsA<UEEnumProperty>())
			{
				auto enumProperty = prop.Cast<UEEnumProperty>();
				//AddDependency(enumProperty.GetEnum().GetPackageObject());
			}
		}
		else if (info.Type == UEProperty::PropertyType::CustomStruct)
		{
			GeneratePrerequisites(prop.Cast<UEStructProperty>().GetStruct(), processedObjects);
		}
		else if (info.Type == UEProperty::PropertyType::Container)
		{
			std::vector<UEProperty> innerProperties;

			if (prop.IsA<UEArrayProperty>())
			{
				innerProperties.push_back(prop.Cast<UEArrayProperty>().GetInner());
			}
			else if (prop.IsA<UEMapProperty>())
			{
				auto mapProp = prop.Cast<UEMapProperty>();
				innerProperties.push_back(mapProp.GetKeyProperty());
				innerProperties.push_back(mapProp.GetValueProperty());
			}

			for (auto innerProp : from(innerProperties)
				>> where([](auto&& p) { return p.GetInfo().Type == UEProperty::PropertyType::CustomStruct; })
				>> cpplinq::experimental::container())
			{
				GeneratePrerequisites(innerProp.Cast<UEStructProperty>().GetStruct(), processedObjects);
			}
		}
		else if (prop.IsA<UEFunction>())
		{
			auto function = prop.Cast<UEFunction>();

			GenerateMemberPrerequisites(function.GetChildren().Cast<UEProperty>(), processedObjects);
		}
	}
}


void Package::PrintStruct(std::ostream& os, const ScriptStruct& ss) const
{
	using namespace cpplinq;

	os << "// " << ss.FullName << "\n// ";
	if (ss.InheritedSize)
	{
		os << tfm::format("0x%04X (0x%04X - 0x%04X)\n", ss.Size - ss.InheritedSize, ss.Size, ss.InheritedSize);
	}
	else
	{
		os << tfm::format("0x%04X\n", ss.Size);
	}

	os << ss.NameCppFull << "\n{\n";

	//Member
	os << (from(ss.Members)
		>> select([](auto&& m) {
			return tfm::format("\t%-50s %-58s// 0x%04X(0x%04X)", m.Type, m.Name + ";", m.Offset, m.Size)
				+ (!m.Comment.empty() ? " " + m.Comment : "")
				+ (!m.FlagsString.empty() ? " (" + m.FlagsString + ")" : "");
			})
		>> concatenate("\n"))
		<< "\n";


			os << "};\n";
}

void Package::PrintEnum(std::ostream& os, const Enum& e) const
{
	using namespace cpplinq;

	os << "// " << e.FullName << "\nenum class " << e.Name << " : uint8_t\n{\n";
	os << (from(e.Values)
		>> select([](auto&& name, auto&& i) { return tfm::format("\t%-30s = %d", name, i); })
		>> concatenate(",\n"))
		<< "\n};\n\n";
}
void Package::PrintClass(std::ostream& os, const Class& c,bool isfunction) const
{
	using namespace cpplinq;

	os << "// " << c.FullName << "\n// ";
	if (c.InheritedSize)
	{
		tfm::format(os, "0x%04X (0x%04X - 0x%04X)\n", c.Size - c.InheritedSize, c.Size, c.InheritedSize);
	}
	else
	{
		tfm::format(os, "0x%04X\n", c.Size);
	}

	os << c.NameCppFull << "\n{\npublic:\n";

	//Member
	for (auto&& m : c.Members)
	{
		tfm::format(os, "\t%-50s %-58s// 0x%04X(0x%04X)", m.Type, m.Name + ";", m.Offset, m.Size);
		if (!m.Comment.empty())
		{
			os << " " << m.Comment;
		}
		if (!m.FlagsString.empty())
		{
			os << " (" << m.FlagsString << ")";
		}
		os << "\n";
	}



		if (!c.Methods.empty() && isfunction)
		{
			os << "\n";
			for (auto&& m : c.Methods)
			{
				os << "\t" << BuildMethodSignature(m, {}, true) << ";\n";
			}
		}

	os << "};\n\n";
}

void Package::PrintFunction(std::ostream& os, const Class& c)
{
	static ULONG64 base = (ULONG64)Process::GetBaseModule();
	//Methods
	if (!c.Methods.empty())
	{
		for (auto&& m : c.Methods)
		{
			os << "///<offset>"<< tfm::format("0x%X", ((ULONG64)m.FunctionAddress - base))<< "</offset>" << endl;
			os << "///<class>"<<m.FullName<< "</class>" << endl;
			os << BuildMethodSignature(m, c, false) << ";\n\n\n";
		}
	}
}

std::string Package::BuildMethodSignature(const Method& m, const Class& c, bool inHeader) const
{
	//extern IGenerator* generator;

	using namespace cpplinq;
	using Type = Method::Parameter::Type;

	std::ostringstream ss;

	if (m.IsStatic && inHeader && !true)
	{
		ss << "static ";
	}

	//Return Type
	auto retn = from(m.Parameters) >> where([](auto&& param) { return param.ParamType == Type::Return; });
	if (retn >> any())
	{
		ss << (retn >> first()).CppType;
	}
	else
	{
		ss << "void";
	}
	ss << " ";

	if (!inHeader)
	{
		ss << c.NameCpp << "::";
	}
	if (m.IsStatic && true)
	{
		ss << "STATIC_";
	}
	//ss << c.NameCppFull;
	ss << m.Name;

	//Parameters
	ss << "(";
	ss << (from(m.Parameters)
		>> where([](auto&& param) { return param.ParamType != Type::Return; })
		>> orderby([](auto&& param) { return param.ParamType; })
		>> select([](auto&& param) { return (param.PassByReference ? "const " : "") + param.CppType + (param.PassByReference ? "& " : param.ParamType == Type::Out ? "* " : " ") + param.Name; })
		>> concatenate(", "));
	ss << ")";

	return ss.str();
}

std::string Package::BuildMethodBody(const Class& c, const Method& m) const
{
	//extern IGenerator* generator;

	using namespace cpplinq;
	using Type = Method::Parameter::Type;

	std::ostringstream ss;

	//Function Pointer
	ss << "{\n\tstatic UFunction* fn;\n";
	ss << "\n\t\tif(!fn)\n";

	if (true)
	{
		ss << "\t\tfn = UObject::FindObject<UFunction>(";

		{
			ss << "\"" << m.FullName << "\"";
		}

		ss << ");\n\n";
	}
	else
	{
		ss << "fn = UObject::GetObjectCasted<UFunction>(" << m.Index << ");\n\n";
	}

	//Parameters
	if (true)
	{
		ss << "\t" << c.NameCpp << "_" << m.Name << "_Params params;\n";
	}
	else
	{
		ss << "\tstruct\n\t{\n";
		for (auto&& param : m.Parameters)
		{
			tfm::format(ss, "\t\t%-30s %s;\n", param.CppType, param.Name);
		}
		ss << "\t} params;\n";
	}

	auto defaultParameters = from(m.Parameters) >> where([](auto&& param) { return param.ParamType == Type::Default; });
	if (defaultParameters >> any())
	{
		for (auto&& param : defaultParameters >> cpplinq::experimental::container())
		{
			ss << "\tparams." << param.Name << " = " << param.Name << ";\n";
		}
	}

	ss << "\n";

	//Function Call
	ss << "\tauto flags = fn->FunctionFlags;\n";
	if (m.IsNative)
	{
		ss << "\tfn->FunctionFlags |= 0x" << tfm::format("%X", static_cast<std::underlying_type_t<UEFunctionFlags>>(UEFunctionFlags::Native)) << ";\n";
	}

	ss << "\n";

	if (m.IsStatic && !true)
	{
		ss << "\tstatic auto defaultObj = StaticClass()->CreateDefaultObject();\n";
		ss << "\tdefaultObj->ProcessEvent(fn, &params);\n\n";
	}
	else
	{
		ss << "\tUObject::ProcessEvent(fn, &params);\n\n";
	}

	ss << "\tfn->FunctionFlags = flags;\n";

	//Out Parameters
	auto out = from(m.Parameters) >> where([](auto&& param) { return param.ParamType == Type::Out; });
	if (out >> any())
	{
		ss << "\n";

		for (auto&& param : out >> cpplinq::experimental::container())
		{
			ss << "\tif (" << param.Name << " != nullptr)\n";
			ss << "\t\t*" << param.Name << " = params." << param.Name << ";\n";
		}
	}

	//Return Value
	auto retn = from(m.Parameters) >> where([](auto&& param) { return param.ParamType == Type::Return; });
	if (retn >> any())
	{
		ss << "\n\treturn params." << (retn >> first()).Name << ";\n";
	}

	ss << "}\n";

	return ss.str();
}


Package::Member Package::CreatePadding(size_t id, size_t offset, size_t size, std::string reason)
{
	Member ss;
	ss.Name = tfm::format("UnknownData%02d[0x%X]", id, size);
	ss.Type = "unsigned char";
	ss.Offset = offset;
	ss.Size = size;
	ss.Comment = std::move(reason);
	return ss;
}

Package::Member Package::CreateBitfieldPadding(size_t id, size_t offset, std::string type, size_t bits)
{
	Member ss;
	ss.Name = tfm::format("UnknownData%02d : %d", id, bits);
	ss.Type = std::move(type);
	ss.Offset = offset;
	ss.Size = 1;
	return ss;
}


void Package::GenerateMethods(const UEClass& classObj, std::vector<Method>& methods) const
{
	//extern IGenerator* generator;

	//some classes (AnimBlueprintGenerated...) have multiple members with the same name, so filter them out
	std::unordered_set<std::string> uniqueMethods;

	for (auto prop = classObj.GetChildren().Cast<UEProperty>(); prop.IsValid(); prop = prop.GetNext().Cast<UEProperty>())
	{
		if (prop.IsA<UEFunction>())
		{
			auto function = prop.Cast<UEFunction>();

			Method m;
			m.Index = function.GetIndex();
			m.FullName = function.GetFullName();
			m.Name = MakeValidName(function.GetName());

			if (uniqueMethods.find(m.FullName) != std::end(uniqueMethods))
			{
				continue;
			}
			uniqueMethods.insert(m.FullName);
			m.FunctionAddress = function.GetFunctionAddress();
			m.IsNative = function.GetFunctionFlags() & UEFunctionFlags::Native;
			m.IsStatic = function.GetFunctionFlags() & UEFunctionFlags::Static;
			m.FlagsString = StringifyFlags(function.GetFunctionFlags());

			std::vector<std::pair<UEProperty, Method::Parameter>> parameters;

			std::unordered_map<std::string, size_t> unique;
			for (auto param = function.GetChildren().Cast<UEProperty>(); param.IsValid(); param = param.GetNext().Cast<UEProperty>())
			{
				if (param.GetElementSize() == 0)
				{
					continue;
				}

				const auto info = param.GetInfo();
				if (info.Type != UEProperty::PropertyType::Unknown)
				{
					using Type = Method::Parameter::Type;

					Method::Parameter p;

					if (!Method::Parameter::MakeType(param.GetPropertyFlags(), p.ParamType))
					{
						//child isn't a parameter
						continue;
					}

					p.PassByReference = false;
					p.Name = MakeValidName(param.GetName());

					const auto it = unique.find(p.Name);
					if (it == std::end(unique))
					{
						unique[p.Name] = 1;
					}
					else
					{
						++unique[p.Name];

						p.Name += tfm::format("%02d", it->second);
					}

					p.FlagsString = StringifyFlags(param.GetPropertyFlags());

					p.CppType = info.CppType;
					if (param.IsA<UEBoolProperty>())
					{
						p.CppType = IGenerator::GetInstance()->GetOverrideType("bool");
					}
					switch (p.ParamType)
					{
					case Type::Default:
						if (prop.GetArrayDim() > 1)
						{
							p.CppType = p.CppType + "*";
						}
						else if (info.CanBeReference)
						{
							p.PassByReference = true;
						}
						break;
					}

					parameters.emplace_back(std::make_pair(prop, std::move(p)));
				}
			}

			std::sort(std::begin(parameters), std::end(parameters), [](auto&& lhs, auto&& rhs) { return ComparePropertyLess(lhs.first, rhs.first); });

			for (auto& param : parameters)
			{
				m.Parameters.emplace_back(std::move(param.second));
			}

			methods.emplace_back(std::move(m));
		}
	}
}




bool Package::Method::Parameter::MakeType(UEPropertyFlags flags, Type& type)
{
	if (flags & UEPropertyFlags::ReturnParm)
	{
		type = Type::Return;
	}
	else if (flags & UEPropertyFlags::OutParm)
	{
		//if it is a const parameter make it a default parameter
		if (flags & UEPropertyFlags::ConstParm)
		{
			type = Type::Default;
		}
		else
		{
			type = Type::Out;
		}
	}
	else if (flags & UEPropertyFlags::Parm)
	{
		type = Type::Default;
	}
	else
	{
		return false;
	}

	return true;
}











void Package::SaveStructs()
{
	using namespace cpplinq;
	if (!constants.empty())
	{
		for (auto&& c : constants) {
			PrintConstant(osStructs, c);
			PrintConstant(osAll, c);

		}

		osStructs << "\n";
		osAll << "\n";
	}

	if (!scriptStructs.empty())
	{
		for (auto&& s : scriptStructs) { 
			PrintStruct(osStructs, s);
			PrintStruct(osAll, s);
			
			osStructs << "\n";
			osAll << "\n";
		}
	}

	osStructs << std::flush;
}


void Package::SaveClasses()
{

	if (!classes.empty())
	{
		for (auto&& c : classes) { 
			PrintClass(osClass, c,false); 
			osClass << "\n"; 
			PrintFunction(osFunction, c);
			PrintClass(osAll, c, true);



		}
	}
	osFunction << std::flush;
	osClass << std::flush;
}

void Package::SvaeEnum()
{
	if (!enums.empty())
	{
		//PrintSectionHeader(os, "Enums");
		for (auto&& e : enums) { 
			PrintEnum(osEnum, e);
			osEnum << "\n"; 
			PrintEnum(osAll, e);
			osAll << "\n";
		
		}

		osEnum << "\n";
	}
	osEnum << std::flush;
}

