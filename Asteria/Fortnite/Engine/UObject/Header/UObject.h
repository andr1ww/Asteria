#pragma once
#include "Framework.h"

namespace Engine::UObject
{
	template<typename T>
	FORCEINLINE static T EncryptPTR(const T Pointer)
	{
		constexpr uint64 PrimaryKey = 
			(UINT64_MAX / 13) * 
			0xFFFFFFFF * 
			__DATE__[5] * 
			0xDADADADA * 
			__TIME__[1] * 
			__TIME__[6] * 
			0xBEEFDEAD * 
			50;
        
		constexpr uint64 SecondaryKey = 
			PrimaryKey * 
			0xFFFFFFFF * 
			__DATE__[5] * 
			0xDADADADA * 
			__TIME__[1] * 
			__TIME__[6] * 
			0xBEEFDEAD * 
			50;

		if constexpr (std::is_pointer_v<T>)
		{
			uint64 ptrValue = reinterpret_cast<uint64>(Pointer);
			return static_cast<T>((ptrValue ^ PrimaryKey) + (0xFFFFFFFF % SecondaryKey));
		}
		else
		{
			return static_cast<T>((static_cast<uint64>(Pointer) ^ PrimaryKey) + (0xFFFFFFFF % SecondaryKey));
		}
	}

	FORCEINLINE static uint64 Find(const std::string& Name)
	{
		return EncryptPTR(reinterpret_cast<uint64>(SDK::UObject::FindObject(Name)));
	}
	
    void ProcessEventHook(SDK::UObject* Object, UFunction* Function, void* Params);
	void Setup();
}