#pragma once
#include "Framework.h"

namespace SDK
{
	inline auto ImageBase = *(uint64_t*)(__readgsqword(0x60) + 0x10);

	class FMemory {
	private:
		static inline auto InternalRealloc = ([]() {
			return (void* (*)(void*, __int64, unsigned int)) Memcury::Scanner::FindPattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC ? 48 8B F1 41 8B D8 48 8B 0D ? ? ? ?").Get();
			})();

	public:
		template <typename _Dt = uint8>
		static _Dt* Malloc(int32 Count, int32 Alignment = alignof(_Dt)) {
			return (_Dt*)InternalRealloc(nullptr, Count * sizeof(_Dt), Alignment);
		}

		template <typename _Dt = uint8>
		static _Dt* Realloc(_Dt* Ptr, int32 Count, int32 Alignment = alignof(std::conditional_t<std::is_void_v<_Dt>, uint8, _Dt>)) {
			if constexpr (std::is_void_v<_Dt>) {
				return (_Dt*)InternalRealloc(Ptr, Count, Alignment);
			} else {
				return (_Dt*)InternalRealloc(Ptr, Count * sizeof(_Dt), Alignment);
			}
		}

		static void Free(void* Ptr) {
			InternalRealloc(Ptr, 0, 0);
		}
	};

	namespace UC
	{
		typedef int8_t  int8;
		typedef int16_t int16;
		typedef int32_t int32;
		typedef int64_t int64;

		typedef uint8_t  uint8;
		typedef uint16_t uint16;
		typedef uint32_t uint32;
		typedef uint64_t uint64;

		template<typename ArrayElementType>
		class TArray;

		template<typename SparseArrayElementType>
		class TSparseArray;

		template<typename SetElementType>
		class TSet;

		template<typename KeyElementType, typename ValueElementType>
		class TMap;

		template<typename KeyElementType, typename ValueElementType>
		class TPair;

		namespace Iterators
		{
			class FSetBitIterator;

			template<typename ArrayType>
			class TArrayIterator;

			template<class ContainerType>
			class TContainerIterator;

			template<typename SparseArrayElementType>
			using TSparseArrayIterator = TContainerIterator<TSparseArray<SparseArrayElementType>>;

			template<typename SetElementType>
			using TSetIterator = TContainerIterator<TSet<SetElementType>>;

			template<typename KeyElementType, typename ValueElementType>
			using TMapIterator = TContainerIterator<TMap<KeyElementType, ValueElementType>>;
		}


		namespace ContainerImpl
		{
			namespace HelperFunctions
			{
				inline uint32 FloorLog2(uint32 Value)
				{
					uint32 pos = 0;
					if (Value >= 1 << 16) { Value >>= 16; pos += 16; }
					if (Value >= 1 << 8) { Value >>= 8; pos += 8; }
					if (Value >= 1 << 4) { Value >>= 4; pos += 4; }
					if (Value >= 1 << 2) { Value >>= 2; pos += 2; }
					if (Value >= 1 << 1) { pos += 1; }
					return pos;
				}

				inline uint32 CountLeadingZeros(uint32 Value)
				{
					if (Value == 0)
						return 32;

					return 31 - FloorLog2(Value);
				}
			}

			template<int32 Size, uint32 Alignment>
			struct TAlignedBytes
			{
				alignas(Alignment) uint8 Pad[Size];
			};

			template<uint32 NumInlineElements>
			class TInlineAllocator
			{
			public:
				template<typename ElementType>
				class ForElementType
				{
				private:
					static constexpr int32 ElementSize = sizeof(ElementType);
					static constexpr int32 ElementAlign = alignof(ElementType);

					static constexpr int32 InlineDataSizeBytes = NumInlineElements * ElementSize;

				private:
					TAlignedBytes<ElementSize, ElementAlign> InlineData[NumInlineElements];
					ElementType* SecondaryData;

				public:
					ForElementType()
						: InlineData{ 0x0 }, SecondaryData(nullptr)
					{
					}

					ForElementType(ForElementType&&) = default;
					ForElementType(const ForElementType&) = default;

				public:
					ForElementType& operator=(ForElementType&&) = default;
					ForElementType& operator=(const ForElementType&) = default;

				public:
					inline const ElementType* GetAllocation() const { return SecondaryData ? SecondaryData : reinterpret_cast<const ElementType*>(&InlineData); }
					inline ElementType* GetAllocation() { return SecondaryData ? SecondaryData : reinterpret_cast<ElementType*>(&InlineData); }

					inline uint32 GetNumInlineBytes() const { return NumInlineElements; }
				};
			};

			class FBitArray
			{
			protected:
				static constexpr int32 NumBitsPerDWORD = 32;
				static constexpr int32 NumBitsPerDWORDLogTwo = 5;

			private:
				TInlineAllocator<4>::ForElementType<int32> Data;
				int32 NumBits;
				int32 MaxBits;

			public:
				FBitArray()
					: NumBits(0), MaxBits(Data.GetNumInlineBytes()* NumBitsPerDWORD)
				{
				}

				FBitArray(const FBitArray&) = default;

				FBitArray(FBitArray&&) = default;

			public:
				FBitArray& operator=(FBitArray&&) = default;

				FBitArray& operator=(const FBitArray& Other) = default;

			private:
				inline void VerifyIndex(int32 Index) const { if (!IsValidIndex(Index)) throw out_of_range("Index was out of range!"); }

			public:
				inline int32 Num() const { return NumBits; }
				inline int32 Max() const { return MaxBits; }

				inline uint32* GetData() { return reinterpret_cast<uint32*>(Data.GetAllocation()); }
				inline const uint32* GetData() const { return reinterpret_cast<const uint32*>(Data.GetAllocation()); }

				inline bool IsValidIndex(int32 Index) const { return Index >= 0 && Index < NumBits; }

				inline bool IsValid() const { return GetData() && NumBits > 0; }

			public:
				inline bool operator[](int32 Index) const { VerifyIndex(Index); return GetData()[Index / NumBitsPerDWORD] & (1 << (Index & (NumBitsPerDWORD - 1))); }

				inline bool operator==(const FBitArray& Other) const { return NumBits == Other.NumBits && GetData() == Other.GetData(); }
				inline bool operator!=(const FBitArray& Other) const { return NumBits != Other.NumBits || GetData() != Other.GetData(); }

				inline void Set(const int32 Index, const bool Value, bool bIsSettingAllZero = false)
				{
					const int32 DWORDIndex = (Index >> ((int32)5));
					const int32 Mask = (1 << (Index & (((int32)32) - 1)));

					if (!bIsSettingAllZero)
						NumBits = Index >= NumBits ? Index < MaxBits ? Index + 1 : NumBits : NumBits;

					Value ? GetData()[DWORDIndex] |= Mask : GetData()[DWORDIndex] &= ~Mask;
				}

			public:
				Iterators::FSetBitIterator begin();
				Iterators::FSetBitIterator end();
			};

			template<typename SparseArrayType>
			union TSparseArrayElementOrFreeListLink
			{
				SparseArrayType ElementData;

				struct
				{
					int32 PrevFreeIndex;
					int32 NextFreeIndex;
				};
			};

			template<typename SetType>
			class SetElement
			{
			private:
				template<typename SetDataType>
				friend class TSet;

			private:
				SetType Value;
				int32 HashNextId;
				int32 HashIndex;
			};
		}


		template <typename KeyType, typename ValueType>
		class TPair
		{
		public:
			KeyType First;
			ValueType Second;

		public:
			TPair(KeyType Key, ValueType Value)
				: First(Key), Second(Value)
			{
			}

		public:
			inline       KeyType& Key() { return First; }
			inline const KeyType& Key() const { return First; }

			inline       ValueType& Value() { return Second; }
			inline const ValueType& Value() const { return Second; }
		};

		template<typename ArrayElementType>
		class TArray
		{
		private:
			template<typename ArrayElementType>
			friend class TAllocatedArray;

			template<typename SparseArrayElementType>
			friend class TSparseArray;

		protected:
			static constexpr uint64 ElementAlign = alignof(ArrayElementType);
			static constexpr uint64 ElementSize = sizeof(ArrayElementType);

		protected:
			ArrayElementType* Data;
			int32 NumElements;
			int32 MaxElements;

		public:
			inline const ArrayElementType* GetPtr(int32 Index, int32 Size) const { VerifyIndex(Index); return (ArrayElementType*)(__int64(Data) + (Index * Size)); }
			inline const ArrayElementType& Get(int32 Index, int32 Size) const { VerifyIndex(Index); return *GetPtr(Index, Size); }
			int GetNumElements() const { return NumElements; }



			TArray()
				: Data(nullptr), NumElements(0), MaxElements(0)
			{
			}

			inline TArray(int32 Size)
				: NumElements(0), MaxElements(Size), Data(reinterpret_cast<ArrayElementType*>(FMemory::Malloc(sizeof(ArrayElementType)* Size, alignof(ArrayElementType))))
			{
			}

			TArray(const TArray&) = default;

			TArray(TArray&&) = default;

		public:
			TArray& operator=(TArray&&) = default;
			TArray& operator=(const TArray&) = default;

		private:
			inline int32 GetSlack() const { return MaxElements - NumElements; }

			inline void VerifyIndex(int32 Index) const { if (!IsValidIndex(Index)) throw out_of_range("Index was out of range!"); }

			inline       ArrayElementType& GetUnsafe(int32 Index) { return Data[Index]; }
			inline const ArrayElementType& GetUnsafe(int32 Index) const { return Data[Index]; }

		public:
			inline void Reserve(const int Num)
			{
				if (Num + NumElements > MaxElements) {
					MaxElements = Num + NumElements;
					Data = static_cast<ArrayElementType*>(FMemory::Realloc(Data, MaxElements * sizeof(ArrayElementType), alignof(ArrayElementType)));
				}
			}

			inline int Add(const ArrayElementType& Element)
			{
				if (NumElements + 1 > MaxElements) {
					MaxElements = NumElements + 1;
					Data = static_cast<ArrayElementType*>(FMemory::Realloc(Data, MaxElements * sizeof(ArrayElementType), alignof(ArrayElementType)));
				}

				memcpy(&Data[NumElements], &Element, sizeof(ArrayElementType));
				NumElements++;

				return NumElements - 1;
			}

			inline bool Remove(int32 Index)
			{
				if (!IsValidIndex(Index))
					return false;

				NumElements--;

				for (int i = Index; i < NumElements; i++)
				{
					/* NumElements was decremented, acessing i + 1 is safe */
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
				NumElements = 0;

				if (!Data)
					memset(Data, 0, NumElements * ElementSize);
			}

		public:
			inline int32 Num() const { return NumElements; }
			inline int32 Max() const { return MaxElements; }

			inline bool IsValidIndex(int32 Index) const { return Data && Index >= 0 && Index < NumElements; }

			inline bool IsValid() const { return Data && NumElements > 0 && MaxElements >= NumElements; }

			ArrayElementType* Search(function<bool(ArrayElementType& val)> check) {
				for (auto& v : *this) {
					if (check(v)) return &v;
				}
				return nullptr;
			}


			int32_t SearchIndex(function<bool(ArrayElementType& val)> check) {
				for (int32_t i = 0; i < Num(); i++) {
					if (check((*this)[i])) return i;
				}
				return -1;
			}

		public:
			inline       ArrayElementType& operator[](int32 Index) { VerifyIndex(Index); return Data[Index]; }
			inline const ArrayElementType& operator[](int32 Index) const { VerifyIndex(Index); return Data[Index]; }

			inline bool operator==(const TArray<ArrayElementType>& Other) const { return Data == Other.Data; }
			inline bool operator!=(const TArray<ArrayElementType>& Other) const { return Data != Other.Data; }

			template <typename NT>
			operator TArray<NT*>() {
				return *(TArray<NT*>*)this;
			}

			inline explicit operator bool() const { return IsValid(); };

		public:
			inline Iterators::TArrayIterator<ArrayElementType> begin() { return Iterators::TArrayIterator<ArrayElementType>(*this, 0); }
			inline Iterators::TArrayIterator<ArrayElementType> end() { return Iterators::TArrayIterator<ArrayElementType>(*this, Num()); }
		};

		class FString : public TArray<wchar_t>
		{
		public:
			friend ostream& operator<<(ostream& Stream, const UC::FString& Str) { return Stream << Str.ToString(); }

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

		/*
		* Class to allow construction of a TArray, that uses c-style standard-library memory allocation.
		*
		* Useful for calling functions that expect a buffer of a certain size and do not reallocate that buffer.
		* This avoids leaking memory, if the array would otherwise be allocated by the engine, and couldn't be freed without FMemory-functions.
		*/
		template<typename ArrayElementType>
		class TAllocatedArray : public TArray<ArrayElementType>
		{
		};

		/*
		* Class to allow construction of an FString, that uses c-style standard-library memory allocation.
		*
		* Useful for calling functions that expect a buffer of a certain size and do not reallocate that buffer.
		* This avoids leaking memory, if the array would otherwise be allocated by the engine, and couldn't be freed without FMemory-functions.
		*/
		class FAllocatedString : public FString
		{
		};

		template<typename SparseArrayElementType>
		class TSparseArray
		{
		private:
			static constexpr uint32 ElementAlign = alignof(SparseArrayElementType);
			static constexpr uint32 ElementSize = sizeof(SparseArrayElementType);

		private:
			using FElementOrFreeListLink = ContainerImpl::TSparseArrayElementOrFreeListLink<ContainerImpl::TAlignedBytes<ElementSize, ElementAlign>>;

		private:
			TArray<FElementOrFreeListLink> Data;
			ContainerImpl::FBitArray AllocationFlags;
			int32 FirstFreeIndex;
			int32 NumFreeIndices;

		public:
			TSparseArray()
				: FirstFreeIndex(-1), NumFreeIndices(0)
			{
			}

			TSparseArray(TSparseArray&&) = default;
			TSparseArray(const TSparseArray&) = default;

		public:
			TSparseArray& operator=(TSparseArray&&) = default;
			TSparseArray& operator=(const TSparseArray&) = default;

		private:
			inline void VerifyIndex(int32 Index) const { if (!IsValidIndex(Index)) throw out_of_range("Index was out of range!"); }

		public:
			inline int32 NumAllocated() const { return Data.Num(); }

			inline int32 Num() const { return NumAllocated() - NumFreeIndices; }
			inline int32 Max() const { return Data.Max(); }

			inline bool IsValidIndex(int32 Index) const { return Data.IsValidIndex(Index) && AllocationFlags[Index]; }

			inline bool IsValid() const { return Data.IsValid() && AllocationFlags.IsValid(); }

			void Remove(int32 Index, int32 Count = 1)
			{
				for (; Count; --Count)
				{
					if (NumFreeIndices) Data[FirstFreeIndex].PrevFreeIndex = Index;
					auto& IndexData = Data[Index];
					IndexData.PrevFreeIndex = -1;
					IndexData.NextFreeIndex = NumFreeIndices > 0 ? FirstFreeIndex : -1;
					FirstFreeIndex = Index;
					++NumFreeIndices;
					AllocationFlags.Set(Index, false);

					++Index;
				}
			}

		public:
			const ContainerImpl::FBitArray& GetAllocationFlags() const { return AllocationFlags; }

		public:
			inline       SparseArrayElementType& operator[](int32 Index) { VerifyIndex(Index); return *reinterpret_cast<SparseArrayElementType*>(&Data.GetUnsafe(Index).ElementData); }
			inline const SparseArrayElementType& operator[](int32 Index) const { VerifyIndex(Index); return *reinterpret_cast<SparseArrayElementType*>(&Data.GetUnsafe(Index).ElementData); }

			inline bool operator==(const TSparseArray<SparseArrayElementType>& Other) const { return Data == Other.Data; }
			inline bool operator!=(const TSparseArray<SparseArrayElementType>& Other) const { return Data != Other.Data; }

		public:
			inline Iterators::TSparseArrayIterator<SparseArrayElementType> begin() { return Iterators::TSparseArrayIterator<SparseArrayElementType>(*this, GetAllocationFlags(), 0); }
			inline Iterators::TSparseArrayIterator<SparseArrayElementType> end() { return Iterators::TSparseArrayIterator<SparseArrayElementType>(*this, GetAllocationFlags(), NumAllocated()); }
		};

		template<typename SetElementType>
		class TSet
		{
		private:
			static constexpr uint32 ElementAlign = alignof(SetElementType);
			static constexpr uint32 ElementSize = sizeof(SetElementType);

		private:
			using SetDataType = ContainerImpl::SetElement<SetElementType>;
			using HashType = ContainerImpl::TInlineAllocator<1>::ForElementType<int32>;

		private:
			TSparseArray<SetDataType> Elements;
			HashType Hash;
			int32 HashSize;

		public:
			TSet()
				: HashSize(0)
			{
			}

			TSet(TSet&&) = default;
			TSet(const TSet&) = default;

		public:
			TSet& operator=(TSet&&) = default;
			TSet& operator=(const TSet&) = default;

		private:
			inline void VerifyIndex(int32 Index) const { if (!IsValidIndex(Index)) throw out_of_range("Index was out of range!"); }

		public:
			inline int32 NumAllocated() const { return Elements.NumAllocated(); }

			inline int32 Num() const { return Elements.Num(); }
			inline int32 Max() const { return Elements.Max(); }

			inline bool IsValidIndex(int32 Index) const { return Elements.IsValidIndex(Index); }

			inline bool IsValid() const { return Elements.IsValid(); }

			inline void Remove(int32 Index) { return Elements.Remove(Index); }

		public:
			const ContainerImpl::FBitArray& GetAllocationFlags() const { return Elements.GetAllocationFlags(); }

		public:
			inline       SetElementType& operator[] (int32 Index) { return Elements[Index].Value; }
			inline const SetElementType& operator[] (int32 Index) const { return Elements[Index].Value; }

			inline bool operator==(const TSet<SetElementType>& Other) const { return Elements == Other.Elements; }
			inline bool operator!=(const TSet<SetElementType>& Other) const { return Elements != Other.Elements; }

		public:
			inline Iterators::TSetIterator<SetElementType> begin() const { return Iterators::TSetIterator<SetElementType>(*this, GetAllocationFlags(), 0); }
			inline Iterators::TSetIterator<SetElementType> end() const { return Iterators::TSetIterator<SetElementType>(*this, GetAllocationFlags(), NumAllocated()); }
		};

		template<typename KeyElementType, typename ValueElementType>
		class TMap
		{
		public:
			using ElementType = TPair<KeyElementType, ValueElementType>;

		private:
			TSet<ElementType> Elements;

		private:
			inline void VerifyIndex(int32 Index) const { if (!IsValidIndex(Index)) throw out_of_range("Index was out of range!"); }

		public:
			inline int32 NumAllocated() const { return Elements.NumAllocated(); }

			inline int32 Num() const { return Elements.Num(); }
			inline int32 Max() const { return Elements.Max(); }

			inline bool IsValidIndex(int32 Index) const { return Elements.IsValidIndex(Index); }

			inline bool IsValid() const { return Elements.IsValid(); }

		public:
			const ContainerImpl::FBitArray& GetAllocationFlags() const { return Elements.GetAllocationFlags(); }

		public:
			inline decltype(auto) Find(const KeyElementType& Key, bool(*Equals)(const KeyElementType& LeftKey, const KeyElementType& RightKey))
			{
				for (auto It = begin(*this); It != end(*this); ++It)
				{
					if (Equals(It->Key(), Key))
						return It;
				}

				return end(*this);
			}

			template<typename NewValueType>
			operator TMap<KeyElementType, NewValueType*>()
			{
				return *(TMap<KeyElementType, NewValueType*> *) this;
			}

			template<typename NewValueType>
			operator TMap<KeyElementType, NewValueType*>() const
			{
				return *(TMap<KeyElementType, NewValueType*> *) this;
			}

		public:
			inline       ElementType& operator[] (int32 Index) { return Elements[Index]; }
			inline const ElementType& operator[] (int32 Index) const { return Elements[Index]; }

			inline bool operator==(const TMap<KeyElementType, ValueElementType>& Other) const { return Elements == Other.Elements; }
			inline bool operator!=(const TMap<KeyElementType, ValueElementType>& Other) const { return Elements != Other.Elements; }

		public:
			inline Iterators::TMapIterator<KeyElementType, ValueElementType> begin() { return Iterators::TMapIterator<KeyElementType, ValueElementType>(*this, GetAllocationFlags(), 0); }
			inline Iterators::TMapIterator<KeyElementType, ValueElementType> end() { return Iterators::TMapIterator<KeyElementType, ValueElementType>(*this, GetAllocationFlags(), NumAllocated()); }
		};

		namespace Iterators
		{
			class FRelativeBitReference
			{
			protected:
				static constexpr int32 NumBitsPerDWORD = 32;
				static constexpr int32 NumBitsPerDWORDLogTwo = 5;

			public:
				inline explicit FRelativeBitReference(int32 BitIndex)
					: WordIndex(BitIndex >> NumBitsPerDWORDLogTwo)
					, Mask(1 << (BitIndex & (NumBitsPerDWORD - 1)))
				{
				}

				int32  WordIndex;
				uint32 Mask;
			};

			class FSetBitIterator : public FRelativeBitReference
			{
			private:
				const ContainerImpl::FBitArray& Array;

				uint32 UnvisitedBitMask;
				int32 CurrentBitIndex;
				int32 BaseBitIndex;

			public:
				explicit FSetBitIterator(const ContainerImpl::FBitArray& InArray, int32 StartIndex = 0)
					: FRelativeBitReference(StartIndex)
					, Array(InArray)
					, UnvisitedBitMask((~0U) << (StartIndex & (NumBitsPerDWORD - 1)))
					, CurrentBitIndex(StartIndex)
					, BaseBitIndex(StartIndex & ~(NumBitsPerDWORD - 1))
				{
					if (StartIndex != Array.Num())
						FindFirstSetBit();
				}

			public:
				inline FSetBitIterator& operator++()
				{
					UnvisitedBitMask &= ~this->Mask;

					FindFirstSetBit();

					return *this;
				}

				inline explicit operator bool() const { return CurrentBitIndex < Array.Num(); }

				inline bool operator==(const FSetBitIterator& Rhs) const { return CurrentBitIndex == Rhs.CurrentBitIndex && &Array == &Rhs.Array; }
				inline bool operator!=(const FSetBitIterator& Rhs) const { return CurrentBitIndex != Rhs.CurrentBitIndex || &Array != &Rhs.Array; }

			public:
				inline int32 GetIndex() { return CurrentBitIndex; }

				void FindFirstSetBit()
				{
					const uint32* ArrayData = Array.GetData();
					const int32   ArrayNum = Array.Num();
					const int32   LastWordIndex = (ArrayNum - 1) / NumBitsPerDWORD;

					uint32 RemainingBitMask = ArrayData[this->WordIndex] & UnvisitedBitMask;
					while (!RemainingBitMask)
					{
						++this->WordIndex;
						BaseBitIndex += NumBitsPerDWORD;
						if (this->WordIndex > LastWordIndex)
						{
							CurrentBitIndex = ArrayNum;
							return;
						}

						RemainingBitMask = ArrayData[this->WordIndex];
						UnvisitedBitMask = ~0;
					}

					const uint32 NewRemainingBitMask = RemainingBitMask & (RemainingBitMask - 1);

					this->Mask = NewRemainingBitMask ^ RemainingBitMask;

					CurrentBitIndex = BaseBitIndex + NumBitsPerDWORD - 1 - ContainerImpl::HelperFunctions::CountLeadingZeros(this->Mask);

					if (CurrentBitIndex > ArrayNum)
						CurrentBitIndex = ArrayNum;
				}
			};

			template<typename ArrayType>
			class TArrayIterator
			{
			private:
				TArray<ArrayType>& IteratedArray;
				int32 Index;

			public:
				TArrayIterator(const TArray<ArrayType>& Array, int32 StartIndex = 0x0)
					: IteratedArray(const_cast<TArray<ArrayType>&>(Array)), Index(StartIndex)
				{
				}

			public:
				inline int32 GetIndex() { return Index; }

				inline int32 IsValid() { return IteratedArray.IsValidIndex(GetIndex()); }

			public:
				inline TArrayIterator& operator++() { ++Index; return *this; }
				inline TArrayIterator& operator--() { --Index; return *this; }

				inline       ArrayType& operator*() { return IteratedArray[GetIndex()]; }
				inline const ArrayType& operator*() const { return IteratedArray[GetIndex()]; }

				inline       ArrayType* operator->() { return &IteratedArray[GetIndex()]; }
				inline const ArrayType* operator->() const { return &IteratedArray[GetIndex()]; }

				inline bool operator==(const TArrayIterator& Other) const { return &IteratedArray == &Other.IteratedArray && Index == Other.Index; }
				inline bool operator!=(const TArrayIterator& Other) const { return &IteratedArray != &Other.IteratedArray || Index != Other.Index; }
			};

			template<class ContainerType>
			class TContainerIterator
			{
			private:
				ContainerType& IteratedContainer;
				FSetBitIterator BitIterator;

			public:
				TContainerIterator(const ContainerType& Container, const ContainerImpl::FBitArray& BitArray, int32 StartIndex = 0x0)
					: IteratedContainer(const_cast<ContainerType&>(Container)), BitIterator(BitArray, StartIndex)
				{
				}

			public:
				inline int32 GetIndex() { return BitIterator.GetIndex(); }

				inline int32 IsValid() { return IteratedContainer.IsValidIndex(GetIndex()); }

			public:
				inline TContainerIterator& operator++() { ++BitIterator; return *this; }
				inline TContainerIterator& operator--() { --BitIterator; return *this; }

				inline       auto& operator*() { return IteratedContainer[GetIndex()]; }
				inline const auto& operator*() const { return IteratedContainer[GetIndex()]; }

				inline       auto* operator->() { return &IteratedContainer[GetIndex()]; }
				inline const auto* operator->() const { return &IteratedContainer[GetIndex()]; }

				inline bool operator==(const TContainerIterator& Other) const { return &IteratedContainer == &Other.IteratedContainer && BitIterator == Other.BitIterator; }
				inline bool operator!=(const TContainerIterator& Other) const { return &IteratedContainer != &Other.IteratedContainer || BitIterator != Other.BitIterator; }
			};
		}

		inline Iterators::FSetBitIterator ContainerImpl::FBitArray::begin() { return Iterators::FSetBitIterator(*this, 0); }
		inline Iterators::FSetBitIterator ContainerImpl::FBitArray::end() { return Iterators::FSetBitIterator(*this, Num()); }

		static_assert(sizeof(TArray<int32>) == 0x10, "TArray has a wrong size!");
		static_assert(sizeof(TSet<int32>) == 0x50, "TSet has a wrong size!");
		static_assert(sizeof(TMap<int32, int32>) == 0x50, "TMap has a wrong size!");
	}

	using namespace UC;


	class FName {
	public:
		int32 ComparisonIndex;
		int32 Number;
		static inline void(*AppendString)(const FName*, UC::FString&) = ([]() {
			auto SRef = Memcury::Scanner::FindStringRef("ForwardShadingQuality_");
			constexpr array<const char*, 5> sigs =
			{
				"48 8D ? ? 48 8D ? ? E8",
				"48 8D ? ? ? 48 8D ? ? E8",
				"48 8D ? ? 49 8B ? E8",
				"48 8D ? ? ? 49 8B ? E8",
				"48 8D ? ? 48 8B ? E8"
			};

			for (auto& sig : sigs) {
				auto Scanner = SRef.ScanFor(sig);
				if (Scanner.Get()) {
					auto pb = Memcury::ASM::pattern2bytes(sig);
					return (void(*)(const FName*, UC::FString&)) Scanner.RelativeOffset((uint32_t)pb.size()).Get();
				}
			}
			return (void(*)(const FName*, UC::FString&)) nullptr;
			})();

		string ToString() const
		{
			thread_local UC::FString TempString(1024);

			AppendString(this, TempString);

			string OutputString = TempString.ToString();
			TempString.Clear();

			return OutputString;
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


	inline double FNVer = ([]() {
		auto FNBuild = Memcury::Scanner::FindPattern("2b 00 2b 00 46 00 6f 00 72 00 74 00 6e 00 69 00 74 00 65 00 2b 00 52 00 65 00 6c 00 65 00 61 00 73 00 65 00 2d 00").Get();
		auto VStart = wcschr((wchar_t*)FNBuild, '-') + 1;
		auto VEnd = wcschr(VStart, '-');
		if (!VEnd) VEnd = (wchar_t*)FNBuild + wcslen((wchar_t*)FNBuild);
		auto sz = (VEnd - VStart) * 2;
		wchar_t* s = (wchar_t*)malloc(sz + 2);
		__movsb((PBYTE)s, (const PBYTE)VStart, sz);
		s[sz] = 0;
		wchar_t* e;
		return wcstod(s, &e);
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
		char _Ch[_Sl + 9];

		consteval DefaultObjChars(const char(&_St)[_Sl])
		{
			copy_n("Default__", 9, _Ch);
			copy_n(_St, _Sl, _Ch + 9);
		}

		operator const char* () const
		{
			return static_cast<const char*>(_Ch);
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

	class UObject {
	public:
		void** Vft;
		int32 ObjectFlags;
		int32 Index;
		class UClass* Class;
		class FName Name;
		UObject* Outer;
		static inline void(*ProcessEventInternal)(const UObject*, class UFunction*, void*) = ([]() {
			uintptr_t addr = Memcury::Scanner::FindStringRef<const wchar_t*>(L"UMeshNetworkComponent::ProcessEvent: Invalid mesh network node type: %s", true, 0, FNVer >= 19.00).ScanFor<1>({ 0xE8 }, true, FNVer < 19.00 ? 1 : 3, 0, 2000).RelativeOffset(1).Get();
			return (void(*)(const UObject*, class UFunction*, void*)) addr;
			})();

	public:
		inline const class UField* GetProperty(const char* Name) const;
		uint32 GetOffset(const char* Name) const {
			static auto OffsetOff = FNVer >= 12.10 && FNVer < 20 ? 0x4c : 0x44;
			auto Prop = GetProperty(Name);
			if (!Prop) return -1;
			return GetFromOffset<uint32>(Prop, OffsetOff);
		}

		template <ConstexprString Name, typename T = UObject*>
		T& Get() const {
			static auto Off = GetOffset(Name);
			if (Off == -1) {
				printf("Failed to get offset for %s!\n", Name._Ch);
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
			static auto Off = GetOffset(Name);
			if (Off == -1) return nullptr;
			return GetPtrFromOffset<T>(this, Off);
		}

		bool IsA(class UClass* Clss) const {
			static auto SuperOff = FNVer >= 7 ? 0x40 : 0x30;
			for (auto _Cl = Class; _Cl; _Cl = GetFromOffset<UClass*>(_Cl, SuperOff)) {
				if (_Cl == Clss) return true;
			}
			return false;
		}

		class UFunction* GetFunction(const char* Name) const {
			return (UFunction*)GetProperty(Name);
		}

		void ProcessEvent(class UFunction* Function, void* Params) const {
			ProcessEventInternal(this, Function, Params);
		}

		template <ConstexprString Name>
		void Call(void* Params = nullptr) const {
			static auto Function = GetFunction(Name);
			ProcessEvent(Function, Params);
		}

		template <ConstexprString Name>
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
			static auto SuperOff = FNVer >= 7 ? 0x40 : 0x30;
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
		const UField* GetProperty(const char* Name) const {
			for (const UClass* _Cl = this; _Cl; _Cl = (const UClass*)_Cl->GetSuper()) {
				if (FNVer >= 12.10) {
					for (const UField* _Pr = _Cl->GetChildren(true); _Pr; _Pr = _Pr->GetNext(true)) {
						if (_Pr->GetName(true).ToSDKString() == Name) return _Pr;
					}
				}
				for (const UField* _Pr = _Cl->GetChildren(false); _Pr; _Pr = _Pr->GetNext(false)) {
					if (_Pr->GetName(false).ToSDKString() == Name) return _Pr;
				}
			}
			return nullptr;
		}
	};


	const UField* UObject::GetProperty(const char* Name) const {
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

	template <ConstexprString Name>
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
			if (Index < 0 || Index > NumElements)
				return nullptr;

			const int32 ChunkIndex = Index / 0x10000;
			const int32 ChunkOffset = Index % 0x10000;

			return Objects[ChunkIndex] + ChunkOffset;
		}
	};

	class TUObjectArray {
	public:
		static inline TUObjectArrayChunked* GObjectsChunked = ([]() {
			if (FNVer >= 5.00) {
				return (TUObjectArrayChunked*)Memcury::Scanner::FindPattern(FNVer <= 6.02 ? "48 8B 05 ? ? ? ? 48 8B 0C C8 48 8D 04 D1" : "48 8B 05 ? ? ? ? 48 8B 0C C8 48 8B 04 D1").RelativeOffset(3).Get();
			}
			return (TUObjectArrayChunked*)nullptr;
			})();
		static inline TUObjectArrayUnchunked* GObjectsUnchunked = ([]() {
			if (FNVer < 5.00) {
				auto Addr = Memcury::Scanner::FindPattern("48 8B 05 ? ? ? ? 48 8D 14 C8 EB 03 49 8B D6 8B 42 08 C1 E8 1D A8 01 0F 85 ? ? ? ? F7 86 ? ? ? ? ? ? ? ?", false);
				if (!Addr.Get()) {
					Addr = Memcury::Scanner::FindPattern("48 8B 05 ? ? ? ? 48 8D 1C C8 81 4B ? ? ? ? ? 49 63 76 30", false);
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

		static const UObject* FindFirstObject(const char* Name) {
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
			return TUObjectArray::FindFirstObject("FortEngine");
		}
	};

	inline EngineWrapper Engine;

	class WorldWrapper : public WrapperBase {
	public:
		const UObject* Get() const {
			return Engine->Get<"GameViewport">()->Get<"World">();
		}
	};

	inline WorldWrapper World;

	class GameModeWrapper : public WrapperBase {
	public:
		const UObject* Get() const {
			return World->Get<"AuthorityGameMode">();
		}
	};

	inline GameModeWrapper GameMode;

	class GameStateWrapper : public WrapperBase {
	public:
		const UObject* Get() const {
			return World->Get<"GameState">();
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

	class FOutputDevice
	{
	public:
		bool bSuppressEventTag;
		bool bAutoEmitLineTerminator;
		uint8_t _Padding1[0x6];
	};

	class FFrame : public FOutputDevice
	{
	public:
		void** VTable;
		UFunction* Node;
		UObject* Object;
		uint8* Code;
		uint8* Locals;
		void* MostRecentProperty;
		uint8_t* MostRecentPropertyAddress;
		uint8_t _Padding1[0x40];
		UField* PropertyChainForCompiledIn;

	public:
		void StepCompiledIn(void* const Result, bool ForceExplicitProp = false)
		{
			if (Code && !ForceExplicitProp)
			{
				((void (*)(FFrame*, UObject*, void* const))(Memcury::Scanner::FindPattern("48 8B 41 20 4C 8B D2 48 8B D1 44 0F B6 08 48 FF").Get()))(this, Object, Result);
			}
			else
			{
				UField* _Prop = PropertyChainForCompiledIn;
				PropertyChainForCompiledIn = const_cast<UField*>(_Prop->GetNext());
				((void (*)(FFrame*, void* const, UField*))(Memcury::Scanner::FindPattern("41 8B 40 ? 4D 8B C8").Get()))(this, Result, _Prop);
			}
		}


		template <typename T>
		T& StepCompiledInRef() {
			T TempVal{};
			MostRecentPropertyAddress = nullptr;

			if (Code)
			{
				((void (*)(FFrame*, UObject*, void* const))(Memcury::Scanner::FindPattern("48 8B 41 20 4C 8B D2 48 8B D1 44 0F B6 08 48 FF").Get()))(this, Object, &TempVal);
			}
			else
			{
				UField* _Prop = PropertyChainForCompiledIn;
				PropertyChainForCompiledIn = const_cast<UField*>(_Prop->GetNext());
				((void (*)(FFrame*, void* const, UField*))(Memcury::Scanner::FindPattern("41 8B 40 ? 4D 8B C8").Get()))(this, &TempVal, _Prop);
			}

			return MostRecentPropertyAddress ? *(T*)MostRecentPropertyAddress : TempVal;
		}

		void IncrementCode() {
			Code = (uint8_t*)(__int64(Code) + (bool)Code);
		}
	};

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

	struct FVector
	{
	public:
		using UnderlayingType = float;                                                                   // 0x0000(0x0008)(NOT AUTO-GENERATED PROPERTY)

		float X;
		float Y;
		float Z;

	public:
		FVector& Normalize()
		{
			*this /= Magnitude();
			return *this;
		}
		FVector& operator*=(const FVector& Other)
		{
			*this = *this * Other;
			return *this;
		}
		FVector& operator*=(float Scalar)
		{
			*this = *this * Scalar;
			return *this;
		}
		FVector& operator+=(const FVector& Other)
		{
			*this = *this + Other;
			return *this;
		}
		FVector& operator-=(const FVector& Other)
		{
			*this = *this - Other;
			return *this;
		}
		FVector& operator/=(const FVector& Other)
		{
			*this = *this / Other;
			return *this;
		}
		FVector& operator/=(float Scalar)
		{
			*this = *this / Scalar;
			return *this;
		}

		UnderlayingType Dot(const FVector& Other) const
		{
			return (X * Other.X) + (Y * Other.Y) + (Z * Other.Z);
		}
		UnderlayingType GetDistanceTo(const FVector& Other) const
		{
			FVector DiffVector = Other - *this;
			return DiffVector.Magnitude();
		}
		UnderlayingType GetDistanceToInMeters(const FVector& Other) const
		{
			return GetDistanceTo(Other) * 0.01f;
		}
		FVector GetNormalized() const
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
		bool operator!=(const FVector& Other) const
		{
			return X != Other.X || Y != Other.Y || Z != Other.Z;
		}
		FVector operator*(const FVector& Other) const
		{
			return { X * Other.X, Y * Other.Y, Z * Other.Z };
		}
		FVector operator*(float Scalar) const
		{
			return { X * Scalar, Y * Scalar, Z * Scalar };
		}
		FVector operator+(const FVector& Other) const
		{
			return { X + Other.X, Y + Other.Y, Z + Other.Z };
		}
		FVector operator-(const FVector& Other) const
		{
			return { X - Other.X, Y - Other.Y, Z - Other.Z };
		}
		FVector operator/(const FVector& Other) const
		{
			if (Other.X == 0.0f || Other.Y == 0.0f || Other.Z == 0.0f)
				return *this;

			return { X / Other.X, Y / Other.Y, Z / Other.Z };
		}
		FVector operator/(float Scalar) const
		{
			if (Scalar == 0.0f)
				return *this;

			return { X / Scalar, Y / Scalar, Z / Scalar };
		}
		bool operator==(const FVector& Other) const
		{
			return X == Other.X && Y == Other.Y && Z == Other.Z;
		}
	};

	struct alignas(0x10) FQuat
	{
		public:
		float X;
		float Y;
		float Z;
		float W;
	};

	struct FRotator final
	{
	public:
		float Pitch;
		float Yaw;
		float Roll;

		operator FQuat() {
			float halfOfARadian = 0.008726646259971648f;
			float sinPitch = sin(Pitch * halfOfARadian),
				sinYaw = sin(Yaw * halfOfARadian),
				sinRoll = sin(Roll * halfOfARadian);
			float cosPitch = cos(Pitch * halfOfARadian),
				cosYaw = cos(Yaw * halfOfARadian),
				cosRoll = cos(Roll * halfOfARadian);

			FQuat out{};
			out.X = cosRoll * sinPitch * sinYaw - sinRoll * cosPitch * cosYaw;
			out.Y = -cosRoll * sinPitch * cosYaw - sinRoll * cosPitch * sinYaw;
			out.Z = cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw;
			out.W = cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw;
			return out;
		}

		operator FVector()
		{
			float oneRadian = 0.017453292519943295f;
			float cosPitch = cos(Pitch * oneRadian), cosYaw = cos(Yaw * oneRadian);
			float sinPitch = sin(Pitch * oneRadian), sinYaw = sin(Yaw * oneRadian);

			return FVector(cosPitch * cosYaw, cosPitch * sinYaw, sinPitch);
		}
	};

	struct FTransform
	{
	public:
		struct FQuat Rotation;
		struct FVector Translation;
		uint8 _Padding1[0x4];
		struct FVector Scale3D;
		uint8 _Padding2[0x4];

		FTransform() {}
		FTransform(FVector loc, FQuat rot, FVector scale = { 1, 1, 1 }) : Translation(loc), Rotation(rot), Scale3D(scale) {}
	};

	class AActor : public UObject {
	public:
		FTransform GetTransform()
		{
			FTransform Ret;
			this->Call<"GetTransform">(&Ret);
			return Ret;
		}
	};

	static TArray<AActor*> GetAll(const UClass* Class) {
		GameplayStatics_GetAllActorsOfClass Params{ World, Class };
		GetDefaultObj<"GameplayStatics">()->Call<"GetAllActorsOfClass">(&Params);
		return Params.OutActors;
	}

	template <ConstexprString Str>
	static TArray<AActor*> GetAll() {
		static auto Class = TUObjectArray::FindObject<UClass>(Str); // we can do this because of the template
		return GetAll(Class);
	}

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

		auto Statics = GetDefaultObj<"GameplayStatics">();
		GameplayStatics_BeginDeferredActorSpawnFromClass Params;
		Params.WorldContextObject = World;
		Params.ActorClass = Class;
		Params.SpawnTransform = Transform;
		Params.CollisionHandlingOverride = 2; // AdjustIfPossibleButAlwaysSpawn
		Params.Owner = Owner;
		Statics->Call<"BeginDeferredActorSpawnFromClass">(&Params);

		GameplayStatics_FinishSpawningActor Params2;
		Params2.Actor = Params.ReturnValue;
		Params2.SpawnTransform = Transform;
		Statics->Call<"FinishSpawningActor">(&Params2);
		return Params2.ReturnValue;
	}

	inline AActor* SpawnActor(const UClass* Class, FTransform Transform, AActor* Owner = nullptr)
	{
		auto Statics = GetDefaultObj<"GameplayStatics">();
		GameplayStatics_BeginDeferredActorSpawnFromClass Params;
		Params.WorldContextObject = World;
		Params.ActorClass = Class;
		Params.SpawnTransform = Transform;
		Params.CollisionHandlingOverride = 2; // AdjustIfPossibleButAlwaysSpawn
		Params.Owner = Owner;
		Statics->Call<"BeginDeferredActorSpawnFromClass">(&Params);

		GameplayStatics_FinishSpawningActor Params2;
		Params2.Actor = Params.ReturnValue;
		Params2.SpawnTransform = Transform;
		Statics->Call<"FinishSpawningActor">(&Params2);
		return Params2.ReturnValue;
	}

	template<typename T = AActor>
	T* SpawnActorUnfinished(const UClass* Class, FVector Loc, FRotator Rot = {}, AActor* Owner = nullptr)
	{
		FTransform Transform(Loc, Rot);

		auto Statics = GetDefaultObj<"GameplayStatics">();
		GameplayStatics_BeginDeferredActorSpawnFromClass Params;
		Params.WorldContextObject = World;
		Params.ActorClass = Class;
		Params.SpawnTransform = Transform;
		Params.CollisionHandlingOverride = 2; // AdjustIfPossibleButAlwaysSpawn
		Params.Owner = Owner;
		Statics->Call<"BeginDeferredActorSpawnFromClass">(&Params);
		return Params.ReturnValue;
	}

	template<typename T = AActor>
	T* FinishSpawnActor(const AActor* Actor, FVector Loc, FRotator Rot)
	{
		FTransform Transform(Loc, Rot);

		auto Statics = GetDefaultObj<"GameplayStatics">();
		GameplayStatics_FinishSpawningActor Params2;
		Params2.Actor = Actor;
		Params2.SpawnTransform = Transform;
		Statics->Call<"FinishSpawningActor">(&Params2);
		return Params2.ReturnValue;
	}

	struct KismetStringLibrary_Conv_StringToName
	{
	public:
		class UC::FString InString;
		class FName ReturnValue;
	};

	inline FName Conv_StringToName(UC::FString Str) {
		KismetStringLibrary_Conv_StringToName Params{ Str };
		GetDefaultObj<"KismetStringLibrary">()->Call<"Conv_StringToName">(&Params);
		return Params.ReturnValue;
	}

	struct UScriptStruct {

	};

	struct FURL
	{
	public:
		class UC::FString Protocol;
		class UC::FString Host;
		int32 Port;
		uint8 Pad_BC4[0x4];
		class UC::FString Map;
		class UC::FString RedirectUrl;
		TArray<class UC::FString> Op;
		class UC::FString Portal;
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
		class UC::FString SubPathString;
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