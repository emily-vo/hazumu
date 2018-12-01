#pragma once
#include <string>
using String = std::string;

class RTTI
{
public:
	virtual const unsigned int& TypeIdInstance() const = 0;

	virtual RTTI* QueryInterface(const unsigned id) const
	{
		return nullptr;
	}

	virtual bool Is(const unsigned int id) const
	{
		return false;
	}

	virtual bool Is(const String& name) const
	{
		return false;
	}

	template <typename T>
	T* As() const
	{
		if (Is(T::TypeIdClass()))
		{
			return (T*)this;
		}

		return nullptr;
	}
};