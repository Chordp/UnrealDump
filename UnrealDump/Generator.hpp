#pragma once
#include "pch.h"
class IGenerator
{
public:
	struct PredefinedMember
	{
		std::string Type;
		std::string Name;
	};
	struct PredefinedMethod
	{
		enum class Type
		{
			Default,
			Inline
		};

		std::string Signature;
		std::string Body;
		Type MethodType;

		/// <summary>Adds a predefined method which gets splittet in declaration and definition.</summary>
		/// <param name="Signature">The method signature.</param>
		/// <param name="Body">The method body.</param>
		/// <returns>The method.</returns>
		static PredefinedMethod Default(std::string&& signature, std::string&& body)
		{
			return { signature, body, Type::Default };
		}

		/// <summary>Adds a predefined method which gets included as an inline method.</summary>
		/// <param name="Body">The body.</param>
		/// <returns>The method.</returns>
		static PredefinedMethod Inline(std::string&& body)
		{
			return { std::string(), body, Type::Inline };
		}
	};
public:
	IGenerator()
	{
	}
	static IGenerator* GetInstance()
	{
		static auto _p = new IGenerator();
		return _p;
	}
	static IGenerator& GetReference()
	{

		return *GetInstance();
	}


	virtual std::string GetOverrideType(const std::string& type) const
	{
		auto it = overrideTypes.find(type);
		if (it == std::end(overrideTypes))
		{
			return type;
		}
		return it->second;
	}




	string GetGameName()
	{
		return "shabi";
	}
protected:
	std::unordered_map<std::string, std::string> overrideTypes;

};