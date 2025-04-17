#include "../Header/Finder.h"

uint64_t Asteria::Finder::FindRequestExit()
{
	auto StringRef = Memcury::Scanner::FindStringRef(skCrypt(L"Memory pools allocations failed, exiting...\n").decrypt());

	if (StringRef.IsValid())
		return StringRef.ScanFor({ Memcury::ASM::CALL }, true, 1).RelativeOffset(1).Get();
	
	return 0;
}

uint64_t Asteria::Finder::FindUnsafeEnvironment()
{
	auto StringRef = Memcury::Scanner::FindStringRef(skCrypt(L"UnsafeEnvironment_Title").decrypt());

	if (StringRef.IsValid())
	{
		for (int i = 0; i < 2048; i++)
		{
			uint8_t* ptr = (uint8_t*)(StringRef.Get() - i);

			if (ptr[0] == 0x4C && ptr[1] == 0x8B && ptr[2] == 0xDC)
				return StringRef.Get() - i;

			if (*(uint16_t*)(StringRef.Get() - i) == 0x5540)
				return StringRef.Get() - i;
		}
	}

	return 0;
}

void Asteria::Finder::GetExit() {
	Asteria::Finder::UnsafeEnvironmentPopup = Asteria::Finder::FindUnsafeEnvironment();
	if (!Asteria::Finder::UnsafeEnvironmentPopup)
	{
		Asteria::Finder::UnsafeEnvironmentPopup = Memcury::Scanner::FindPattern("4C 8B DC 55 49 8D AB ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ?").Get();
		if (!Asteria::Finder::UnsafeEnvironmentPopup)
			Asteria::Finder::UnsafeEnvironmentPopup = Memcury::Scanner::FindPattern("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 80 B9 ? ? ? ? ? 48 8B DA 48 8B F1").Get();
	}
	if (!Asteria::Finder::UnsafeEnvironmentPopup)
	{
		Log("Failed to find UnsafeEnvironmentPopupAddr");
	}

	Asteria::Finder::RequestExit = Asteria::Finder::FindRequestExit();
	if (!Asteria::Finder::RequestExit)
	{
		Log("Failed to find RequestExitAddr");
	}
}

void Asteria::Finder::InitializeExitHook()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(reinterpret_cast<void**>(&UnsafeEnvironmentPopup), Asteria::Finder::UnsafeEnvironmentPopupHook);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(reinterpret_cast<void**>(&RequestExit), Asteria::Finder::RequestExitHook);
	DetourTransactionCommit();
}