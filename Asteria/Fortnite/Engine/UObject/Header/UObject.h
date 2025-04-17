#pragma once
#include "Framework.h"

namespace Engine::UObject
{
	template<typename T>
	FORCEINLINE static T EncryptPTR(const T Pointer)
	{
		constexpr uint64 Primary = (UINT64_MAX / 13) * 0xFFFFFFFF * __DATE__[5] * 0xDADADADA * __TIME__[1] * __TIME__[6] * 0xBEEFDEAD * 50;
		constexpr uint64 Secondary = Primary * 0xFFFFFFFF * __DATE__[5] * 0xDADADADA * __TIME__[1] * __TIME__[6] * 0xBEEFDEAD * 50;

		return (T)((uint64)Pointer ^ Primary + 0xFFFFFFFF % Secondary);
	}

	FORCEINLINE static uint64 Find(const std::string& Name)
	{
		return EncryptPTR(reinterpret_cast<uint64>(SDK::UObject::FindObject(Name)));
	}
	
	void ProcessEventHook(SDK::UObject* Object, UFunction* Function, void* Params);
	void Setup();
}