#pragma once
#include <string>
using String = std::string;
#define RTTI_DECLARATIONS(Type, ParentType)                                                              \
        public:                                                                                              \
            static std::string TypeName() { return std::string(#Type); }                                     \
            virtual const unsigned int& TypeIdInstance() const { return Type::TypeIdClass(); }               \
            static  const unsigned int& TypeIdClass() { return sRunTimeTypeId; }                             \
            virtual RTTI* QueryInterface( const unsigned int id ) const                             \
            {                                                                                                \
                if (id == sRunTimeTypeId)                                                                    \
                    { return (RTTI*)this; }                                                                  \
                else                                                                                         \
					{ return ParentType::QueryInterface(id); }                                               \
            }                                                                                                \
            virtual bool Is(const unsigned int id) const                                                     \
            {                                                                                                \
                if (id == sRunTimeTypeId)                                                                    \
                    { return true; }                                                                         \
                else                                                                                         \
                    { return ParentType::Is(id); }                                                           \
            }                                                                                                \
            virtual bool Is(const std::string& name) const                                                   \
            {                                                                                                \
                if (name == TypeName())                                                                      \
                    { return true; }                                                                         \
                else                                                                                         \
                    { return ParentType::Is(name); }                                                         \
            }                                                                                                \
	   private:                                                                                              \
            static unsigned int sRunTimeTypeId;

#define RTTI_DEFINITIONS(Type) unsigned int Type::sRunTimeTypeId = (unsigned int)& Type::sRunTimeTypeId;


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