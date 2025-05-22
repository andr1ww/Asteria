#pragma once
#include "Framework.h"

template <typename _Ft>
struct ConstexprFunc {
	_Ft _Fn;

public:
	consteval ConstexprFunc(_Ft _Pf)
	{
		_Fn = _Pf;
	}

	constexpr _Ft Get() const {
		return _Fn;
	}
};

template <size_t _Sz>
struct ConstexprArray {
	uint8_t _Ar[_Sz];
public:
	consteval ConstexprArray(std::array<uint8_t, _Sz> _Pa) {
		std::copy_n(_Pa.data(), _Sz, _Ar);
	}

	constexpr void* Get() const {
		return (void*)_Ar;
	}
};

template<int32 _Sl>
struct ConstexprString
{
	char _Ch[_Sl];

	consteval ConstexprString(const char(&_St)[_Sl])
	{
		copy_n(_St, _Sl, _Ch);
	}

	operator const char* () const
	{
		return static_cast<const char*>(_Ch);
	}


	constexpr std::string_view StringView() const {
		return _Ch;
	}

	constexpr int PatternCount() const {
		int c = 0;
		for (int i = 0; i < _Sl; i++) {
			if (_Ch[i] == ' ') c++;
		}
		return c + 1; // last i think
	}
};


template<int32 _Sl>
struct ConstexprWString
{
	wchar_t _Ch[_Sl];

	consteval ConstexprWString(const wchar_t(&_St)[_Sl])
	{
		copy_n(_St, _Sl, _Ch);
	}

	operator const wchar_t* () const
	{
		return static_cast<const wchar_t*>(_Ch);
	}
};


namespace SDK
{
	
	class FMemory {
	public:
		static inline auto InternalRealloc = ([]() {
			return (void* (*)(void*, __int64, unsigned int)) Memcury::Scanner::FindPattern(_("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC ? 48 8B F1 41 8B D8 48 8B 0D ? ? ? ?").decrypt()).Get();
			})();

	public:
		template <typename _Dt = uint8>
		static _Dt* Malloc(int32 Count, int32 Alignment = alignof(_Dt)) {
			return (_Dt*)InternalRealloc(nullptr, Count * sizeof(_Dt), Alignment);
		}

		template <typename _Dt = uint8>
		static _Dt* Realloc(_Dt* Ptr, int32 Count, int32 Alignment = alignof(_Dt)) {
			return (_Dt*)InternalRealloc(Ptr, Count * sizeof(_Dt), Alignment);
		}

		static void Free(void* Ptr) {
			InternalRealloc(Ptr, 0, 0);
		}
	};

	template <typename _Et>
	class TArray {
	public:
		_Et* Data;
		int32 NumElements;
		int32 MaxElements;

	private:
		inline int32 GetSlack() const { return MaxElements - NumElements; }

		inline void VerifyIndex(int32 Index) const { if (!IsValidIndex(Index)) throw out_of_range("Index was out of range!"); }

		inline       _Et& GetUnsafe(int32 Index) { return Data[Index]; }
		inline const _Et& GetUnsafe(int32 Index) const { return Data[Index]; }


	public:
		TArray() : Data(nullptr), NumElements(0), MaxElements(0)
		{
		}

		inline TArray(int32 Size) : NumElements(0), MaxElements(Size), Data(FMemory::Malloc<_Et>(Size))
		{
		}

		inline void Reserve(const int Num)
		{
			if (Num > GetSlack()) Data = FMemory::Realloc<_Et>(Data, MaxElements = Num + NumElements);
		}

		inline _Et& Add(const _Et& Element)
		{
			Reserve(1);

			auto& Elem = Data[NumElements] = Element;
			NumElements++;

			return Elem;
		}

		inline bool Remove(int32 Index)
		{
			if (!IsValidIndex(Index))
				return false;

			NumElements--;

			for (int i = Index; i < NumElements; i++)
			{
				Data[i] = Data[i + 1];
			}

			return true;
		}

		inline void Free()
		{
			if (Data)
				FMemory::Free(Data);

			MaxElements = 0;
			NumElements = 0;
			Data = nullptr;
		}

		inline void ResetNum() {
			NumElements = 0;
		}

		inline void Clear()
		{
			ResetNum();

			if (Data) __stosb((PBYTE)Data, 0, NumElements * sizeof(_Et));
		}

	public:
		inline int32 Num() const { return NumElements; }
		inline int32 Max() const { return MaxElements; }

		inline bool IsValidIndex(int32 Index) const { return Data && Index >= 0 && Index < NumElements; }

		inline bool IsValid() const { return Data && NumElements > 0 && MaxElements >= NumElements; }

		_Et* Search(function<bool(_Et& val)> check) {
			for (auto& v : *this) {
				if (check(v)) return &v;
			}
			return nullptr;
		}


		int32_t SearchIndex(function<bool(_Et& val)> check) {
			for (int32_t i = 0; i < Num(); i++) {
				if (check((*this)[i])) return i;
			}
			return -1;
		}

	public:
		inline       _Et& operator[](int32 Index) { VerifyIndex(Index); return Data[Index]; }
		inline const _Et& operator[](int32 Index) const { VerifyIndex(Index); return Data[Index]; }


		inline const _Et* GetPtr(int32 Index, int32 Size) const { VerifyIndex(Index); return (_Et*)(__int64(Data) + (Index * Size)); }
		inline const _Et& Get(int32 Index, int32 Size) const { VerifyIndex(Index); return *GetPtr(Index, Size); }

		inline explicit operator bool() const { return IsValid(); };

	public:
		inline _Et* begin() { return Data; }
		inline _Et* end() { return Data + Num(); }
	};

	class SDKFString : public TArray<wchar_t> {
	public:
		using TArray::TArray;

		SDKFString(const wchar_t* Str)
		{
			const uint32 Len = static_cast<uint32>(wcslen(Str) + 0x1);

			Data = const_cast<wchar_t*>(Str);
			MaxElements = NumElements = Len;
		}

	public:
		inline string ToString() const
		{
			if (*this)
			{
				wstring _Wd(Data);
#pragma warning(suppress: 4244)
				return string(_Wd.begin(), _Wd.end());
			}

			return "";
		}

	public:
		inline       wchar_t* c_str() { return Data; }
		inline const wchar_t* c_str() const { return Data; }

	public:
		inline bool operator==(const SDKFString& Other) const { return Other && NumElements == Other.NumElements ? wcscmp(Data, Other.Data) == 0 : false; }
		inline bool operator!=(const SDKFString& Other) const { return Other && NumElements != Other.NumElements ? wcscmp(Data, Other.Data) != 0 : true; }
	};

	class TName {
	public:
		int32 ComparisonIndex;

		static inline void(*AppendString)(const TName*, SDKFString&) = ([]() {
			auto SRef = Memcury::Scanner::FindStringRef<const char*>(_("ForwardShadingQuality_").decrypt());

	#define kys(_Sz, ...) {\
		auto Scanner = SRef.ScanFor<_Sz>({ ##__VA_ARGS__ }); \
		if (Scanner.Get()) { \
		return (void(*)(const TName*, SDKFString&)) Scanner.RelativeOffset(_Sz).Get(); \
		} \
		}

			kys(9, 0x48, 0x8D, 0xFF, 0xFF, 0x48, 0x8D, 0xFF, 0xFF, 0xE8);
			kys(10, 0x48, 0x8D, 0xFF, 0xFF, 0xFF, 0x48, 0x8D, 0xFF, 0xFF, 0xE8);
			kys(8, 0x48, 0x8D, 0xFF, 0xFF, 0x49, 0x8B, 0xFF, 0xE8);
			kys(9, 0x48, 0x8D, 0xFF, 0xFF, 0xFF, 0x49, 0x8B, 0xFF, 0xE8);
			kys(8, 0x48, 0x8D, 0xFF, 0xFF, 0x48, 0x8B, 0xFF, 0xE8);
			return (void(*)(const TName*, SDKFString&)) nullptr;
			})();

		string ToString() const
		{
			thread_local SDKFString TName_TempString = SDKFString(1024);
			AppendString(this, TName_TempString);

			string OutputString = TName_TempString.ToString();
			TName_TempString.Clear();

			return OutputString;
		}


		wstring ToWString() const
		{
			thread_local SDKFString TName_TempString = SDKFString(1024);
			AppendString(this, TName_TempString);

			wstring OutputString = TName_TempString.c_str();
			TName_TempString.Clear();

			return OutputString;
		}

		wchar_t* GetWStr() const
		{
			thread_local SDKFString TName_TempString = SDKFString(1024);
			TName_TempString.Clear();
			AppendString(this, TName_TempString);

			return TName_TempString.c_str();
		}


		wchar_t* GetSDKWStr() const
		{
			wchar_t *OutputString = GetWStr();

			wchar_t* pos = wcsrchr(OutputString, L'/');
		
			if (!pos)
				return OutputString;

			return pos + 1;
		}

		string ToSDKString() const
		{
			std::string OutputString = ToString();

			size_t pos = OutputString.rfind('/');

			if (pos == std::string::npos)
				return OutputString;

			return OutputString.substr(pos + 1);
		}
	};

	class FName : public TName
	{
	public:
		int32 Number;
	};


	class FNameLWC : public TName
	{
	};

	inline double FNVer = ([]() {
		auto FNBuild = Memcury::Scanner::FindPattern(_("2b 00 2b 00 46 00 6f 00 72 00 74 00 6e 00 69 00 74 00 65 00 2b 00 52 00 65 00 6c 00 65 00 61 00 73 00 65 00 2d 00").decrypt()).Get();
		if (!FNBuild)
		{
			return 69.42;
		}
		auto VStart = wcschr((wchar_t*)FNBuild, '-') + 1;
		auto VEnd = wcschr(VStart, '-');
		if (!VEnd) VEnd = (wchar_t*)FNBuild + wcslen((wchar_t*)FNBuild);
		auto sz = (VEnd - VStart) * 2;
		wchar_t* s = (wchar_t*)LI_FN(malloc).in_cached(GetModuleHandleA(_("api-ms-win-crt-heap-l1-1-0.dll").decrypt()))(sz + 2);
		__movsb((PBYTE)s, (const PBYTE)VStart, sz);
		s[sz] = 0;
		wchar_t* e;
		return LI_FN(wcstod).in_cached(GetModuleHandleA(_("api-ms-win-crt-convert-l1-1-0.dll").decrypt()))(s, &e);
		})();

	inline int Chapter = ([]() {
		if (FNVer >= 28.00) return 5;
		if (FNVer == 27.00) return 1; // does c4sog use athena?
		if (FNVer >= 23.00) return 4;
		if (FNVer >= 19.00) return 3;
		if (FNVer >= 11.00) return 2;
		return 1;
		})();


	template <typename _Ot>
	_Ot& GetFromOffset(void* Obj, uint32 Offset) {
		return *(_Ot*)(__int64(Obj) + Offset);
	}

	template <typename _Ot>
	_Ot& GetFromOffset(const void* Obj, uint32 Offset) {
		return *(_Ot*)(__int64(Obj) + Offset);
	}

	template <typename _Ot>
	_Ot* GetPtrFromOffset(const void* Obj, uint32 Offset) {
		return (_Ot*)(__int64(Obj) + Offset);
	}


	template<int32 _Sl>
	struct DefaultObjChars
	{
		wchar_t _Ch[_Sl + 9];

		consteval DefaultObjChars(const wchar_t(&_St)[_Sl])
		{
			copy_n(L"Default__", 9, _Ch);
			copy_n(_St, _Sl, _Ch + 9);
		}

		operator const wchar_t* () const
		{
			return static_cast<const wchar_t*>(_Ch);
		}
	};

	class ParamPair {
	public:
		std::string _Fi;
		void* _Sc;

		template <typename _Vt>
		ParamPair(std::string _Nm, _Vt _Va) {
			_Fi = _Nm;
			// really scuffed way to make this work, just using & gives the same address for each param
			_Sc = FMemory::Malloc(sizeof(_Vt));
			memcpy(_Sc, &_Va, sizeof(_Vt));
		}
	};


	class FStructBaseChain
	{
	public:
		FStructBaseChain()
			: StructBaseChainArray(nullptr)
			, NumStructBasesInChainMinusOne(-1)
		{
		}
		~FStructBaseChain()
		{
			delete[] StructBaseChainArray;
		}

		FStructBaseChain(const FStructBaseChain&) = delete;
		FStructBaseChain& operator=(const FStructBaseChain&) = delete;

		__forceinline bool IsChildOfUsingStructArray(const FStructBaseChain& Parent) const
		{
			int32 NumParentStructBasesInChainMinusOne = Parent.NumStructBasesInChainMinusOne;
			return NumParentStructBasesInChainMinusOne <= NumStructBasesInChainMinusOne && StructBaseChainArray[NumParentStructBasesInChainMinusOne] == &Parent;
		}

	private:
		FStructBaseChain** StructBaseChainArray;
		int32 NumStructBasesInChainMinusOne;

		friend class UStruct;
	};


	class UObject {
	public:
		void** Vft;
		int32 ObjectFlags;
		int32 Index;
		class UClass* Class;
		class FName Name;
		UObject* Outer;
		static inline void(*ProcessEventInternal)(const UObject*, class UFunction*, void*) = ([]() {
	uintptr_t addr;
			if (FNVer < 14.00)
				addr = Memcury::Scanner::FindStringRef<const wchar_t*>(_(L"AccessNoneNoContext").decrypt()).ScanFor<2>({ 0x40, 0x55 }, true, 0, 1, 2000).Get();
			else
				addr = Memcury::Scanner::FindStringRef<const wchar_t*>(_(L"UMeshNetworkComponent::ProcessEvent: Invalid mesh network node type: %s").decrypt(), true, 0, FNVer >= 19.00).ScanFor<1>({ 0xE8 }, true, FNVer < 19.00 ? 1 : 3, 0, 2000).RelativeOffset(1).Get();
	return (void(*)(const UObject*, class UFunction*, void*)) addr;
	})();

	public:
		const class UField* GetProperty(const wchar_t* Name) const;
		uint32 GetOffset(const wchar_t* Name) const {
			auto OffsetOff = FNVer >= 12.10 && FNVer < 20 ? 0x4c : 0x44;
			auto Prop = GetProperty(Name);
			if (!Prop) return -1;
			return GetFromOffset<uint32>(Prop, OffsetOff);
		}

		template <ConstexprWString Name, typename T = UObject*>
		T& Get() const {
			auto Off = GetOffset(Name);
			if (Off == -1) {
				//printf("Failed to get offset for %ls!\n", Name._Ch);
				while (true);
			}
			return GetFromOffset<T>(this, Off);
		}


		template <typename T = UObject*>
		T& Get(const wchar_t *Name) const {
			auto Off = GetOffset(Name);
			if (Off == -1) {
				while (true);
			}
			return GetFromOffset<T>(this, Off);
		}

		template <ConstexprString Name>
		void SetBitfield(bool Value) const {
			static auto OffsetOff = FNVer >= 12.10 && FNVer < 20 ? 0x4c : 0x44;
			static auto Prop = GetProperty(Name);
			uint8& Field = GetFromOffset<uint8>(Prop, OffsetOff);
			if (Value) {
				Field |= Prop->GetFieldMask();
			}
			else {
				Field &= ~Prop->GetFieldMask();
			}
		}

		template <ConstexprString Name>
		bool Has() const {
			return GetOffset(Name) != -1;
		}

		template <ConstexprString Name, typename T = UObject*>
		T* GetPtr() const {
			auto Off = GetOffset(Name);
			if (Off == -1) return nullptr;
			return GetPtrFromOffset<T>(this, Off);
		}

		bool IsA(const class UClass* Clss) const {
			if (!this || !Clss)
				return false;
			if (FNVer >= 7)
			{
				auto& BaseChain = GetFromOffset<FStructBaseChain>(Class, 0x30);
				auto& BaseChainOther = GetFromOffset<FStructBaseChain>(Clss, 0x30);

				return BaseChain.IsChildOfUsingStructArray(BaseChainOther);
			}
			auto SuperOff = FNVer >= 7 ? 0x40 : 0x30;
			for (auto _Cl = Class; _Cl; _Cl = GetFromOffset<UClass*>(_Cl, SuperOff)) {
				if (_Cl == Clss) return true;
			}
			return false;
		}

		UObject* Cast(const class UClass* Class)
		{
			return IsA(Class) ? this : nullptr;
		}

		class UFunction* GetFunction(const wchar_t* Name) const {
			return (UFunction*)GetProperty(Name);
		}

		void ProcessEvent(class UFunction* Function, void* Params) const {
			ProcessEventInternal(this, Function, Params);
		}

		template <ConstexprWString Name>
		void Call(void* Params = nullptr) const {
			auto Function = GetFunction(Name);
			ProcessEvent(Function, Params);
		}

		void Call(const wchar_t *Name, void* Params = nullptr) const {
			auto Function = GetFunction(Name);
			ProcessEvent(Function, Params);
		}

		template <ConstexprWString Name>
		void Call(std::vector<ParamPair> Params) const;
	};

	class UField : public UObject {
	public:
		const UField* GetNext(bool bNewFields = false) const {
			auto NextOff = bNewFields ? 0x20 : 0x28;
			return GetFromOffset<UField*>(this, NextOff);
		}


		FName& GetName(bool bNewFields = false) const {
			auto NameOff = bNewFields ? 0x28 : 0x18;
			return GetFromOffset<FName>(this, NameOff);
		}

		const uint8 GetFieldMask() const {
			static auto FieldMaskOff = FNVer >= 12.21 && FNVer < 20 ? 0x7b : 0x73;
			return GetFromOffset<uint8>(this, FieldMaskOff);
		}
	};

	class UStruct : public UField {
	public:
		const UStruct* GetSuper() const {
			auto SuperOff = FNVer >= 7 ? 0x40 : 0x30;
			return GetFromOffset<UStruct*>(this, SuperOff);
		}
		const int32 GetPropertiesSize() const {
			auto ChildrenOff = FNVer >= 12.10 ? 0x58 : (FNVer >= 7 ? 0x50 : 0x40);
			return GetFromOffset<int32>(this, ChildrenOff);
		}

		const UField* GetChildren(bool bNewFields = false) const {
			auto ChildrenOff = bNewFields ? 0x50 : (FNVer >= 7 ? 0x48 : 0x38);
			return GetFromOffset<UField*>(this, ChildrenOff);
		}
	};

	class UClass : public UStruct {
	public:
		const UField* GetProperty(const wchar_t* Name) const {
			for (const UClass* _Cl = this; _Cl; _Cl = (const UClass*)_Cl->GetSuper()) {
				if (FNVer >= 12.10) {
					for (const UField* _Pr = _Cl->GetChildren(true); _Pr; _Pr = _Pr->GetNext(true))
						if (wcscmp(_Pr->GetName(true).GetSDKWStr(), Name) == 0) return _Pr;
				}
				for (const UField* _Pr = _Cl->GetChildren(false); _Pr; _Pr = _Pr->GetNext(false))
					if (wcscmp(_Pr->GetName(false).GetSDKWStr(), Name) == 0) return _Pr;
			}
			return nullptr;
		}
	};


	inline const UField* UObject::GetProperty(const wchar_t* Name) const {
		return Class->GetProperty(Name);
	}

	class UFunction : public UStruct {
	public:
		void Call(const UObject* obj, void* Params) {
			if (this) obj->ProcessEvent(this, Params);
		}

		void Call(const UObject* obj, std::vector<ParamPair> Params) {
			if (this) obj->ProcessEvent(this, CreateParams(Params));
		}

		void operator()(const UObject* obj, void* Params) {
			return Call(obj, Params);
		}

		void*& GetNativeFunc() {
			if (FNVer <= 6.31) return GetFromOffset<void*>(this, 0xB0);
			else if (FNVer > 7.00 && FNVer < 12.00) return GetFromOffset<void*>(this, 0xC0);
			else if (FNVer >= 12.00 && FNVer < 12.10) return GetFromOffset<void*>(this, 0xC8);
			else if (FNVer >= 12.10 && FNVer <= 12.61) return GetFromOffset<void*>(this, 0xF0);
			else return GetFromOffset<void*>(this, 0xD8);
		}

		int32 GetVTableIndex() {
			if (!this) return -1;
			auto ValidateName = Name.ToString() + "_Validate";
			auto ValidateRef = Memcury::Scanner::FindStringRef(std::wstring(ValidateName.begin(), ValidateName.end()).c_str(), false);

			auto Addr = ValidateRef.Get();

			if (!Addr) {
				Addr = __int64(GetNativeFunc());
			}

			if (Addr) {
				for (int i = 0; i < 2000; i++) {
					if (*((uint8*)Addr + i) == 0xFF && (*((uint8*)Addr + i + 1) == 0x90 || *((uint8*)Addr + i + 1) == 0x93 || *((uint8*)Addr + i + 1) == 0xA0)) {
						auto VTIndex = *(uint32_t*)(Addr + i + 2);

						return VTIndex / 8;
					}
				}
			}
			return -1;
		}

		struct Param {
			std::string Name;
			uint32 Offset;
		};
		class Params {
		public:
			std::vector<Param> NameOffsetMap;
			uint32 Size;
		};

		Params GetParams() {
			Params p;
			static auto OffsetOff = FNVer >= 12.10 && FNVer < 20 ? 0x4c : 0x44;

			if (FNVer >= 12.10) {
				for (const UField* _Pr = GetChildren(true); _Pr; _Pr = _Pr->GetNext(true)) {
					p.NameOffsetMap.push_back({ _Pr->GetName(true).ToSDKString(), GetFromOffset<uint32>(_Pr, OffsetOff) });
				}
			}
			for (const UField* _Pr = GetChildren(false); _Pr; _Pr = _Pr->GetNext(false)) {
				p.NameOffsetMap.push_back({ _Pr->GetName(false).ToSDKString(), GetFromOffset<uint32>(_Pr, OffsetOff) });
			}

			p.Size = GetPropertiesSize();
			return p;
		}

		void* CreateParams(std::vector<ParamPair> InputParams) {
			auto Mem = FMemory::Malloc(GetParams().Size);
			auto Params = GetParams();

			for (auto& _Pa : InputParams) {
				Param FoundParam;
				int i = 0;
				uint32 Size = 0;
				for (auto& _Mp : Params.NameOffsetMap) {
					if (_Mp.Name == _Pa._Fi) {
						FoundParam = _Mp;
						Size = i == Params.NameOffsetMap.size() - 1 ? Params.Size - _Mp.Offset : Params.NameOffsetMap[i + 1].Offset - _Mp.Offset;
						break;
					}
					i++;
				}

				if (Size) {
					memcpy((PBYTE)Mem + FoundParam.Offset, _Pa._Sc, Size);
				}
			}
			return Mem;
		}

		template <typename _Rt>
		_Rt* GetValueFromParams(void* Params, const char* Name) {
			auto Params = GetParams();

			for (auto& _Mp : Params.NameOffsetMap) {
				if (_Mp.Name == Name) {
					return GetPtrFromOffset<_Rt>(Params, _Mp.Offset);
				}
			}

			return nullptr;
		}
	};

	template <ConstexprWString Name>
	void UObject::Call(std::vector<ParamPair> Params) const {
		static auto Function = GetFunction(Name);
		ProcessEvent(Function, Function->CreateParams(Params));
	}

	struct FUObjectItem final
	{
	public:
		class UObject* Object;
		int32 Flags;
		int32 ClusterRootIndex;
		int32 SerialNumber;
	};

	class TUObjectArrayUnchunked
	{
	private:
		const FUObjectItem* Objects;
		const int32 MaxElements;
		const int32 NumElements;

	public:
		inline int Num() const
		{
			return NumElements;
		}

		inline int Max() const
		{
			return MaxElements;
		}

		inline const FUObjectItem* GetItemByIndex(const int32 Index) const
		{
			if (Index < 0 || Index > NumElements)
				return nullptr;

			return Objects + Index;
		}
	};

	class TUObjectArrayChunked
	{
	private:
		const FUObjectItem** Objects;
		const FUObjectItem* PreAllocatedObjects;
		const int32 MaxElements;
		const int32 NumElements;
		const int32 MaxChunks;
		const int32 NumChunks;

	public:
		inline int Num() const
		{
			return NumElements;
		}

		inline int Max() const
		{
			return MaxElements;
		}

		inline const FUObjectItem* GetItemByIndex(const int32 Index) const
		{
			const int32 ChunkIndex = Index / 0x10000;
			const int32 ChunkOffset = Index % 0x10000;

			return Objects[ChunkIndex] + ChunkOffset;
		}
	};

	class TUObjectArray {
	public:
		static inline TUObjectArrayChunked* GObjectsChunked = ([]() {
			if (FNVer >= 5.00) {
				return (TUObjectArrayChunked*)Memcury::Scanner::FindPattern(FNVer <= 6.02 ? _("48 8B 05 ? ? ? ? 48 8B 0C C8 48 8D 04 D1").decrypt() : _("48 8B 05 ? ? ? ? 48 8B 0C C8 48 8B 04 D1").decrypt()).RelativeOffset(3).Get();
			}
			return (TUObjectArrayChunked*)nullptr;
			})();
		static inline TUObjectArrayUnchunked* GObjectsUnchunked = ([]() {
			if (FNVer < 5.00) {
				auto Addr = Memcury::Scanner::FindPattern(_("48 8B 05 ? ? ? ? 48 8D 14 C8 EB 03 49 8B D6 8B 42 08 C1 E8 1D A8 01 0F 85 ? ? ? ? F7 86 ? ? ? ? ? ? ? ?"), false);
				if (!Addr.Get()) {
					Addr = Memcury::Scanner::FindPattern(_("48 8B 05 ? ? ? ? 48 8D 1C C8 81 4B ? ? ? ? ? 49 63 76 30").decrypt(), false);
				}
				return (TUObjectArrayUnchunked*)Addr.RelativeOffset(3).Get();
			}
			return (TUObjectArrayUnchunked*)nullptr;
			})();
	public:
		static const int32 Num() {
			return GObjectsChunked ? GObjectsChunked->Num() : GObjectsUnchunked->Num();
		}

		static const int32 Max() {
			return GObjectsChunked ? GObjectsChunked->Max() : GObjectsUnchunked->Max();
		}

		static const FUObjectItem* GetItemByIndex(const int32 Index) {
			return GObjectsChunked ? GObjectsChunked->GetItemByIndex(Index) : GObjectsUnchunked->GetItemByIndex(Index);
		}

		static const UObject* GetObjectByIndex(const int32 Index) {
			const FUObjectItem* Item = GetItemByIndex(Index);
			return Item ? Item->Object : nullptr;
		}

		static const UObject* FindObject(const char* Name) {
			for (int i = 0; i < Num(); i++) {
				const UObject* Obj = GetObjectByIndex(i);
				if (Obj && Obj->Name.ToString() == Name)
					return Obj;
			}
			return nullptr;
		}

		template <typename _Et = UObject>
		static const _Et* FindObject(const char* Name) {
			return (const _Et*)FindObject(Name);
		}

		template <typename _Et = UObject>
		static const _Et* FindObject(const std::string& Name) {
			return FindObject<_Et>(Name.c_str());
		}

		static const UObject* FindObject(const wchar_t* Name) {
			for (int i = 0; i < Num(); i++) {
				const UObject* Obj = GetObjectByIndex(i);
				if (Obj && wcscmp(Obj->Name.GetWStr(), Name) == 0) 
					return Obj;
			}
			return nullptr;
		}

		template <typename _Et = UObject>
		static const _Et* FindObject(const wchar_t* Name) {
			return (const _Et*)FindObject(Name);
		}

		template <typename _Et = UObject>
		static const _Et* FindObject(const std::wstring& Name) {
			return FindObject<_Et>(Name.c_str());
		}

		static const UObject* FindFirstObject(const wchar_t* Name) {
			UClass* TargetClass = (UClass*)FindObject(Name);
			for (int i = 0; i < Num(); i++) {
				const UObject* Obj = GetObjectByIndex(i);
				if (Obj && !(Obj->ObjectFlags & 16) && Obj->IsA(TargetClass))
					return Obj;
			}
			return nullptr;
		}
	};

	template <typename T = UObject*, typename _St>
	T& StructGet(_St*& StructInstance, const char* StructName, const char* Name) {
		auto Struct = TUObjectArray::FindObject<UClass>(StructName);

		static auto OffsetOff = FNVer >= 12.10 && FNVer < 20 ? 0x4c : 0x44;
		auto Off = GetFromOffset<uint32>(Struct->GetProperty(Name), OffsetOff);
		return GetFromOffset<T>(StructInstance, Off);
	}

	template <DefaultObjChars Name>
	const UObject* GetDefaultObj() {
		static auto Obj = TUObjectArray::FindObject(Name);
		return Obj;
	}

	class WrapperBase {
	private:
		virtual const UObject* Get() const = 0;

	public:
		operator const UObject* () const {
			return Get();
		}

		operator UObject* () const {
			return (UObject*)Get();
		}

		const UObject* operator->() const {
			return Get();
		}

		operator bool() const {
			return Get() != nullptr;
		}
	};

	class EngineWrapper : public WrapperBase {
	public:
		const UObject* Get() const {
			return TUObjectArray::FindFirstObject(_(L"FortEngine").decrypt());
		}
	};

	inline EngineWrapper Engine;

	class WorldWrapper : public WrapperBase {
	public:
		const UObject* Get() const {
			auto _Engine = Engine.Get();
			if (!_Engine) 
				return nullptr;

			auto VPOffset = _Engine->GetOffset(_(L"GameViewport").decrypt());
			if (VPOffset == -1)
				return nullptr;

			auto _Viewport = GetFromOffset<UObject*>(_Engine, VPOffset);
			if (!_Viewport)
				return nullptr;

			auto WOffset = _Viewport->GetOffset(_(L"World").decrypt());
			if (WOffset == -1)
				return nullptr;

			return GetFromOffset<UObject*>(_Viewport, WOffset);
		}
	};

	inline WorldWrapper World;

	class GameModeWrapper : public WrapperBase {
	public:
		const UObject* Get() const {
			auto _Wr = World.Get();
			if (!_Wr)
				return nullptr;

			auto GMOffset = _Wr->GetOffset(_(L"AuthorityGameMode").decrypt());
			if (GMOffset == -1)
				return nullptr;
		

			return GetFromOffset<UObject*>(_Wr, GMOffset);
		}
	};

	inline GameModeWrapper GameMode;

	class GameStateWrapper : public WrapperBase {
	public:
		const UObject* Get() const {
			return World ? World->Get(_(L"GameState").decrypt()) : nullptr;
		}
	};

	inline GameStateWrapper GameState;

	inline void* nullptrForHook;

	template<DefaultObjChars ClassName, typename T = void*>
	__forceinline void Hook(int idx, void* detour, T& og = nullptrForHook) {
		auto VTable = (void**)GetDefaultObj<ClassName>()->Vft;
		if (!is_same_v<T, void*>)
			og = (T)VTable[idx];

		DWORD vpog;
		VirtualProtect(VTable + idx, 8, PAGE_EXECUTE_READWRITE, &vpog);
		VTable[idx] = detour;
		VirtualProtect(VTable + idx, 8, vpog, &vpog);
	}


	template<DefaultObjChars ClassName, ConstexprString FuncName, typename T = void*>
	__forceinline void Hook(void* detour, T& og = nullptrForHook) {
		auto DefaultObj = GetDefaultObj<ClassName>();
		auto VTable = (void**)DefaultObj->Vft;
		int idx = DefaultObj->GetFunction(FuncName)->GetVTableIndex();
		if (!is_same_v<T, void*>)
			og = (T)VTable[idx];

		DWORD vpog;
		VirtualProtect(VTable + idx, 8, PAGE_EXECUTE_READWRITE, &vpog);
		VTable[idx] = detour;
		VirtualProtect(VTable + idx, 8, vpog, &vpog);
	}

	struct FFastArraySerializerItem
	{
	public:
		int32 ReplicationID;
		int32 ReplicationKey;
		int32 MostRecentArrayReplicationKey;
	};

	struct alignas(0x08) FFastArraySerializer
	{
		public:
		char ItemMap[0x50];
		int32 IDCounter;
		int32 ArrayReplicationKey;
		char GuidReferencesMap[0x50];

		int32& GetCachedItems() {
			return GetFromOffset<int32>(this, FNVer >= 8.30 ? 0xf8 : 0xa8);
		}

		int32& GetCachedItemsToConsiderForWriting() {
			return GetFromOffset<int32>(this, FNVer >= 8.30 ? 0xfc : 0xac);
		}

		/** This must be called if you add or change an item in the array */
		void MarkItemDirty(FFastArraySerializerItem& Item)
		{
			if (Item.ReplicationID == -1)
			{
				Item.ReplicationID = ++IDCounter;
				if (IDCounter == -1)
				{
					IDCounter++;
				}
			}

			Item.ReplicationKey++;
			MarkArrayDirty();
		}

		/** This must be called if you just remove something from the array */
		void MarkArrayDirty()
		{
			// ItemMap.Reset();        // This allows to clients to add predictive elements to arrays without affecting replication.
			IncrementArrayReplicationKey();

			// Invalidate the cached item counts so that they're recomputed during the next write
			GetCachedItems() = -1;
			GetCachedItemsToConsiderForWriting() = -1;
		}

		void IncrementArrayReplicationKey()
		{
			ArrayReplicationKey++;
			if (ArrayReplicationKey == -1)
			{
				ArrayReplicationKey++;
			}
		}
	};

	class AActor : public UObject {};

	template<typename _Ct>
	class TSubclassOf
	{
		const UClass* ClassPtr;

	public:
		TSubclassOf() = default;

		inline TSubclassOf(const UClass* Class)
			: ClassPtr(Class)
		{
		}

		inline UClass* Get()
		{
			return ClassPtr;
		}

		inline operator UClass* () const
		{
			return ClassPtr;
		}

		template<typename Target, typename = std::enable_if<std::is_base_of_v<Target, _Ct>, bool>::type>
		inline operator TSubclassOf<Target>() const
		{
			return ClassPtr;
		}

		inline UClass* operator->()
		{
			return ClassPtr;
		}

		inline TSubclassOf& operator=(UClass* Class)
		{
			ClassPtr = Class;

			return *this;
		}

		inline bool operator==(const TSubclassOf& Other) const
		{
			return ClassPtr == Other.ClassPtr;
		}

		inline bool operator!=(const TSubclassOf& Other) const
		{
			return ClassPtr != Other.ClassPtr;
		}

		inline bool operator==(UClass* Other) const
		{
			return ClassPtr == Other;
		}

		inline bool operator!=(UClass* Other) const
		{
			return ClassPtr != Other;
		}
	};

	struct GameplayStatics_GetAllActorsOfClass final
	{
	public:
		const class UObject* WorldContextObject;
		TSubclassOf<class AActor> ActorClass;
		TArray<class AActor*> OutActors;
	};


	static TArray<AActor*> GetAll(const UClass* Class) {
		GameplayStatics_GetAllActorsOfClass Params{ World, Class };
		GetDefaultObj<L"GameplayStatics">()->Call<L"GetAllActorsOfClass">(&Params);
		return Params.OutActors;
	}

	template <ConstexprString Str>
	static TArray<AActor*> GetAll() {
		static auto Class = TUObjectArray::FindObject<UClass>(Str); // we can do this because of the template
		return GetAll(Class);
	}

	template <typename UnderlayingType>
	struct TVector
	{
	public:
		UnderlayingType X;
		UnderlayingType Y;
		UnderlayingType Z;

	public:
		TVector(UnderlayingType _X = 0, UnderlayingType _Y = 0, UnderlayingType _Z = 0)
			: X(_X), Y(_Y), Z(_Z)
		{
		}

		TVector<UnderlayingType>& Normalize()
		{
			*this /= Magnitude();
			return *this;
		}
		TVector<UnderlayingType>& operator*=(const TVector<UnderlayingType>& Other)
		{
			*this = *this * Other;
			return *this;
		}
		TVector<UnderlayingType>& operator*=(UnderlayingType Scalar)
		{
			*this = *this * Scalar;
			return *this;
		}
		TVector<UnderlayingType>& operator+=(const TVector<UnderlayingType>& Other)
		{
			*this = *this + Other;
			return *this;
		}
		TVector<UnderlayingType>& operator-=(const TVector<UnderlayingType>& Other)
		{
			*this = *this - Other;
			return *this;
		}
		TVector<UnderlayingType>& operator/=(const TVector<UnderlayingType>& Other)
		{
			*this = *this / Other;
			return *this;
		}
		TVector<UnderlayingType>& operator/=(UnderlayingType Scalar)
		{
			*this = *this / Scalar;
			return *this;
		}

		UnderlayingType Dot(const TVector<UnderlayingType>& Other) const
		{
			return (X * Other.X) + (Y * Other.Y) + (Z * Other.Z);
		}
		UnderlayingType GetDistanceTo(const TVector<UnderlayingType>& Other) const
		{
			TVector<UnderlayingType> DiffVector = Other - *this;
			return DiffVector.Magnitude();
		}
		UnderlayingType GetDistanceToInMeters(const TVector<UnderlayingType>& Other) const
		{
			return GetDistanceTo(Other) * 0.01f;
		}
		TVector<UnderlayingType> GetNormalized() const
		{
			return *this / Magnitude();
		}
		bool IsZero() const
		{
			return X == 0.0 && Y == 0.0 && Z == 0.0;
		}
		UnderlayingType Magnitude() const
		{
			return std::sqrt((X * X) + (Y * Y) + (Z * Z));
		}
		bool operator!=(const TVector<UnderlayingType>& Other) const
		{
			return X != Other.X || Y != Other.Y || Z != Other.Z;
		}
		TVector<UnderlayingType> operator*(const TVector<UnderlayingType>& Other) const
		{
			return { X * Other.X, Y * Other.Y, Z * Other.Z };
		}
		TVector<UnderlayingType> operator*(UnderlayingType Scalar) const
		{
			return { X * Scalar, Y * Scalar, Z * Scalar };
		}
		TVector<UnderlayingType> operator+(const TVector<UnderlayingType>& Other) const
		{
			return { X + Other.X, Y + Other.Y, Z + Other.Z };
		}
		TVector<UnderlayingType> operator-(const TVector<UnderlayingType>& Other) const
		{
			return { X - Other.X, Y - Other.Y, Z - Other.Z };
		}
		TVector<UnderlayingType> operator/(const TVector<UnderlayingType>& Other) const
		{
			if (Other.X == 0.0f || Other.Y == 0.0f || Other.Z == 0.0f)
				return *this;

			return { X / Other.X, Y / Other.Y, Z / Other.Z };
		}
		TVector<UnderlayingType> operator/(UnderlayingType Scalar) const
		{
			if (Scalar == 0.0f)
				return *this;

			return { X / Scalar, Y / Scalar, Z / Scalar };
		}
		bool operator==(const TVector<UnderlayingType>& Other) const
		{
			return X == Other.X && Y == Other.Y && Z == Other.Z;
		}
	};

	struct FVector : public TVector<float> 
	{
		using TVector::TVector;
	};
	struct FVectorLWC : public TVector<double> 
	{
		using TVector::TVector;
	};

	template <typename UnderlayingType>
	struct alignas(0x10) TQuat
	{
		public:
		UnderlayingType X;
		UnderlayingType Y;
		UnderlayingType Z;
		UnderlayingType W;
	};

	struct FQuat : public TQuat<float>
	{
	};
	struct FQuatLWC : public TQuat<double>
	{
	};

	template <typename UnderlayingType>
	struct TRotator
	{
	public:
		UnderlayingType Pitch;
		UnderlayingType Yaw;
		UnderlayingType Roll;

		operator TQuat<UnderlayingType>() {
			UnderlayingType halfOfARadian = (UnderlayingType) 0.008726646259971648;
			UnderlayingType sinPitch = sin(Pitch * halfOfARadian),
				sinYaw = sin(Yaw * halfOfARadian),
				sinRoll = sin(Roll * halfOfARadian);
			UnderlayingType cosPitch = cos(Pitch * halfOfARadian),
				cosYaw = cos(Yaw * halfOfARadian),
				cosRoll = cos(Roll * halfOfARadian);

			TQuat<UnderlayingType> out{};
			out.X = cosRoll * sinPitch * sinYaw - sinRoll * cosPitch * cosYaw;
			out.Y = -cosRoll * sinPitch * cosYaw - sinRoll * cosPitch * sinYaw;
			out.Z = cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw;
			out.W = cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw;
			return out;
		}

		operator TVector<UnderlayingType>()
		{
			UnderlayingType oneRadian = (UnderlayingType) 0.017453292519943295;
			UnderlayingType cosPitch = cos(Pitch * oneRadian), cosYaw = cos(Yaw * oneRadian);
			UnderlayingType sinPitch = sin(Pitch * oneRadian), sinYaw = sin(Yaw * oneRadian);

			return TVector<UnderlayingType>(cosPitch * cosYaw, cosPitch * sinYaw, sinPitch);
		}
	};


	struct FRotator : public TRotator<float>
	{
	};
	struct FRotatorLWC : public TRotator<double>
	{
	};

	template <typename UnderlayingType>
	struct TTransform
	{
	public:
		struct TQuat<UnderlayingType> Rotation;
		struct TVector<UnderlayingType> Translation;
		uint8 _Padding1[0x4];
		struct TVector<UnderlayingType> Scale3D;
		uint8 _Padding2[0x4];

		TTransform() {}
		TTransform(TVector<UnderlayingType> loc, TQuat<UnderlayingType> rot, TVector<UnderlayingType> scale = { 1, 1, 1 }) : Translation(loc), Rotation(rot), Scale3D(scale) {}
	};

	struct FTransform : public TTransform<float>
	{
		using TTransform::TTransform;
	};
	struct FTransformLWC : public TTransform<double>
	{
		using TTransform::TTransform;
	};

	struct GameplayStatics_BeginDeferredActorSpawnFromClass
	{
	public:
		const class UObject* WorldContextObject;
		TSubclassOf<class AActor> ActorClass;
		struct FTransform SpawnTransform;
		uint8 CollisionHandlingOverride;
		uint8 _Padding1[0x7];
		class AActor* Owner;
		class AActor* ReturnValue;
		uint8 _Padding2[0x8];
	};

	struct GameplayStatics_FinishSpawningActor
	{
	public:
		class AActor* Actor;
		uint8 _Padding1[0x8];
		struct FTransform SpawnTransform;
		class AActor* ReturnValue;
		uint8 _Padding2[0x8];
	};

	inline AActor* SpawnActor(const UClass* Class, FVector Loc, FRotator Rot = {}, AActor* Owner = nullptr)
	{
		FTransform Transform(Loc, Rot);

		auto Statics = GetDefaultObj<L"GameplayStatics">();
		GameplayStatics_BeginDeferredActorSpawnFromClass Params;
		Params.WorldContextObject = World;
		Params.ActorClass = Class;
		Params.SpawnTransform = Transform;
		Params.CollisionHandlingOverride = 2; // AdjustIfPossibleButAlwaysSpawn
		Params.Owner = Owner;
		Statics->Call<L"BeginDeferredActorSpawnFromClass">(&Params);

		GameplayStatics_FinishSpawningActor Params2;
		Params2.Actor = Params.ReturnValue;
		Params2.SpawnTransform = Transform;
		Statics->Call<L"FinishSpawningActor">(&Params2);
		return Params2.ReturnValue;
	}

	inline AActor* SpawnActor(const UClass* Class, FTransform Transform, AActor* Owner = nullptr)
	{
		auto Statics = GetDefaultObj<L"GameplayStatics">();
		GameplayStatics_BeginDeferredActorSpawnFromClass Params;
		Params.WorldContextObject = World;
		Params.ActorClass = Class;
		Params.SpawnTransform = Transform;
		Params.CollisionHandlingOverride = 2; // AdjustIfPossibleButAlwaysSpawn
		Params.Owner = Owner;
		Statics->Call<L"BeginDeferredActorSpawnFromClass">(&Params);

		GameplayStatics_FinishSpawningActor Params2;
		Params2.Actor = Params.ReturnValue;
		Params2.SpawnTransform = Transform;
		Statics->Call<L"FinishSpawningActor">(&Params2);
		return Params2.ReturnValue;
	}

	template<typename T = AActor>
	T* SpawnActorUnfinished(const UClass* Class, FVector Loc, FRotator Rot = {}, AActor* Owner = nullptr)
	{
		FTransform Transform(Loc, Rot);

		auto Statics = GetDefaultObj<L"GameplayStatics">();
		GameplayStatics_BeginDeferredActorSpawnFromClass Params;
		Params.WorldContextObject = World;
		Params.ActorClass = Class;
		Params.SpawnTransform = Transform;
		Params.CollisionHandlingOverride = 2; // AdjustIfPossibleButAlwaysSpawn
		Params.Owner = Owner;
		Statics->Call<L"BeginDeferredActorSpawnFromClass">(&Params);
		return Params.ReturnValue;
	}

	class FString : public TArray<wchar_t>
	{
	public:
		friend ostream& operator<<(ostream& Stream, const FString& Str) { return Stream << Str.ToString(); }

	public:
		using TArray::TArray;

		FString(const wchar_t* Str)
		{
			const uint32 NullTerminatedLength = static_cast<uint32>(wcslen(Str) + 0x1);

			Data = const_cast<wchar_t*>(Str);
			NumElements = NullTerminatedLength;
			MaxElements = NullTerminatedLength;
		}

	public:
		inline string ToString() const
		{
			if (*this)
			{
				wstring WData(Data);
#pragma warning(suppress: 4244)
				return string(WData.begin(), WData.end());
			}

			return "";
		}

		inline wstring ToWString() const
		{
			if (*this)
				return wstring(Data);

			return L"";
		}

	public:
		inline       wchar_t* CStr() { return Data; }
		inline const wchar_t* CStr() const { return Data; }

	public:
		inline bool operator==(const FString& Other) const { return Other ? NumElements == Other.NumElements && wcscmp(Data, Other.Data) == 0 : false; }
		inline bool operator!=(const FString& Other) const { return Other ? NumElements != Other.NumElements || wcscmp(Data, Other.Data) != 0 : true; }
	};

	
	struct KismetStringLibrary_Conv_StringToName
	{
	public:
		class FString InString;
		class FName ReturnValue;
	};

	inline FName Conv_StringToName(FString Str) {
		KismetStringLibrary_Conv_StringToName Params{ Str };
		TUObjectArray::FindObject(_(L"Default__KismetStringLibrary").decrypt())->Call(_(L"Conv_StringToName").decrypt(), &Params);
		return Params.ReturnValue;
	}

	struct UScriptStruct {

	};

	struct FURL
	{
	public:
		class FString Protocol;
		class FString Host;
		int32 Port;
		uint8 Pad_BC4[0x4];
		class FString Map;
		class FString RedirectUrl;
		TArray<class FString> Op;
		class FString Portal;
		int32 Valid;
		uint8 Pad_BC5[0x4];
	};

	class FWeakObjectPtr
	{
	public:
		int32 ObjectIndex;
		int32 ObjectSerialNumber;

	public:
		const UObject* Get() const {
			return TUObjectArray::GetObjectByIndex(ObjectIndex);
		}
		const UObject* operator->() const {
			return Get();
		}
		bool operator==(const FWeakObjectPtr& Other) const {
			return ObjectIndex == Other.ObjectIndex;
		}
		bool operator!=(const FWeakObjectPtr& Other) const {
			return ObjectIndex != Other.ObjectIndex;
		}
		bool operator==(const class UObject* Other) const {
			return ObjectIndex == Other->Index;
		}
		bool operator!=(const class UObject* Other) const {
			return ObjectIndex != Other->Index;
		}
	};

	template<typename _Oi>
	class TPersistentObjectPtr
	{
	public:
		FWeakObjectPtr WeakPtr;
		int32 TagAtLastTest;
		_Oi ObjectID;

	public:
		class UObject* Get() const
		{
			return WeakPtr.Get();
		}
		class UObject* operator->() const
		{
			return WeakPtr.Get();
		}
	};


	struct FSoftObjectPath
	{
	public:
		class FName AssetPathName;
		class FString SubPathString;
	};

	template<typename _Ut>
	class TSoftObjectPtr : public TPersistentObjectPtr<FSoftObjectPath>
	{
	public:
		_Ut* Get() const
		{
			return static_cast<_Ut*>(TPersistentObjectPtr::Get());
		}
		_Ut* operator->() const
		{
			return static_cast<_Ut*>(TPersistentObjectPtr::Get());
		}
	};

	template<typename _Ut>
	class TSoftClassPtr : public TPersistentObjectPtr<FSoftObjectPath>
	{
	public:
		_Ut* Get() const
		{
			return static_cast<_Ut*>(TPersistentObjectPtr::Get());
		}
		_Ut* operator->() const
		{
			return static_cast<_Ut*>(TPersistentObjectPtr::Get());
		}
	};

	struct FUIExtension final
	{
	public:
		uint8                                         Slot;                                              // 0x0000(0x0001)(Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		uint8                                         Pad_1[0x7];                                        // 0x0001(0x0007)(Fixing Size After Last Property [ Dumper-7 ])
		TSoftClassPtr<class UClass>                   WidgetClass;                                       // 0x0008(0x0028)(Edit, DisableEditOnInstance, UObjectWrapper, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	};
}