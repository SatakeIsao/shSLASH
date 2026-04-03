/**
 * Memory.h
 * メモリ周りの処理を書くファイル
 */
#pragma once
#include "memory/Allocator.h"


namespace app
{
	namespace memory
	{
		/**
		 * ===================================================
		 * 独自のアロケーターを使用するための処理群
		 * ===================================================
		 */

		/** Heap用 (Global) */
#if defined(APP_ENABLE_DEBUG_ALLOCATOR)
		template <typename T, typename... Args>
		T* New(const char* file, int line, Args&&... args)
		{
			void* ptr = Allocator::Get().Allocate(sizeof(T), file, line, typeid(T).name());
			return ptr ? new(ptr) T(std::forward<Args>(args)...) : nullptr;
		}
#else
		template <typename T, typename... Args>
		T* New(Args&&... args)
		{
			void* ptr = Allocator::Get().Allocate(sizeof(T));
			return ptr ? new(ptr) T(std::forward<Args>(args)...) : nullptr;
		}
#endif // APP_ENABLE_DEBUG_ALLOCATOR
		template <typename T>
		void Delete(T* ptr)
		{
			if (ptr) { ptr->~T(); Allocator::Get().Free(ptr); }
		}

#if defined(APP_ENABLE_DEBUG_ALLOCATOR)
#define appNew(Type, ...) New<Type>(__FILE__, __LINE__, ##__VA_ARGS__)
#else
#define appNew(Type, ...) New<Type>(##__VA_ARGS__)
#endif // APP_ENABLE_DEBUG_ALLOCATOR
#define appDelete(Ptr)    Delete(Ptr)
#define appAlloc(Size)    Allocator::Get().Allocate(Size, __FILE__, __LINE__, "RawBlock")
#define appFree(Ptr)      Allocator::Get().Free(Ptr)


		/** 固定配列を扱いやすくしたクラス */
		template <typename T>
		class Array
		{
		public:
			/** C++標準コンテナに合わせた型定義 */
			using value_type = T;
			using size_type = size_t;
			using difference_type = ptrdiff_t;
			using pointer = T*;
			using const_pointer = const T*;
			using reference = T&;
			using const_reference = const T&;

			/** 連続メモリなので、ポインタそのものがイテレータとして機能する */
			using iterator = T*;
			using const_iterator = const T*;


		private:
			/** 生ポインタ */
			T* m_data = nullptr;
			size_t m_size = 0;


		public:
			/** コンストラクタ */
			Array() = default;
			/** デストラクタ(生ポインタなので明示的にdelete[]が必要) */
			~Array()
			{
				Release();
			}

			/**
			 * 生成関数
			 * NOTE: ここで初めてメモリ確保
			 */
#if defined(APP_ENABLE_DEBUG_ALLOCATOR)
			void Create(size_t size, const char* file = __FILE__, int line = __LINE__)
#else
			void Create(size_t size)
#endif // APP_ENABLE_DEBUG_ALLOCATOR
			{ 
				K2_ASSERT(size > 0, "サイズは0より大きい値を指定してください");
				// 既に持っているなら解放
				Release();

				if (size > 0) {
					// 生メモリの確保 (独自アロケーター経由)
#if defined(APP_ENABLE_DEBUG_ALLOCATOR)
					void* ptr = Allocator::Get().Allocate(sizeof(T) * size, file, line, "Array");
#else
					void* ptr = Allocator::Get().Allocate(sizeof(T) * size);
#endif
					m_data = static_cast<T*>(ptr);
					m_size = size;
					// コンストラクタ呼び出し
					// 確保したメモリ上の各要素に対して初期化を行う
					for (size_t i = 0; i < m_size; ++i) {
						new (&m_data[i]) T();
					}
				}
			}


			/**
			 * 解放関数
			 * デストラクタ呼び出しとメモリ解放を行います
			 */
			void Release()
			{
				if (m_data) {
					// 1. デストラクタ呼び出し
					for (size_t i = m_size; i > 0; --i) {
						m_data[i - 1].~T();
					}

					// 2. メモリ解放 (独自アロケーター経由)
					Allocator::Get().Free(m_data);

					m_data = nullptr;
					m_size = 0;
				}
			}




			/** コピー禁止 */
			Array(const Array&) = delete;
			Array& operator=(const Array&) = delete;

			/**
			 * ムーブ (所有権移動) は許可
			 */
			Array(Array&& other) noexcept
				: m_data(other.m_data), m_size(other.m_size)
			{
				// 生ポインタ管理なので、移動元のポインタをnullptrにする処理が必須
				other.m_data = nullptr;
				other.m_size = 0;
			}

			/** 代入 */
			Array& operator=(Array&& other) noexcept
			{
				if (this != & other) {
					// 自分のリソースを解放
					if (m_data) {
						delete[] m_data;
					}
					// 所有権を奪う
					m_data = other.m_data;
					m_size = other.m_size;

					// 移動元を空にする
					other.m_data = nullptr;
					other.m_size = 0;
				}
				return *this;
			}




			/**
			 * アクセス関数関連
			 */

			 /**
			  * 生ポインタ取得
			  * NOTE: std::vectorのdata()関数のようなもの
			  */
			T* data() noexcept { return m_data; }
			const T* data() const noexcept { return m_data; }

			/** 配列アクセス */
			T& operator[](size_t index) { return m_data[index]; }
			const T& operator[](size_t index) const { return m_data[index]; }

			/** サイズ取得 */
			size_t size() const noexcept { return m_size; }
			bool empty() const noexcept { return m_size == 0; }

			/** イテレータ (ポインタを返す) */
			iterator begin() noexcept { return m_data; }
			iterator end() noexcept { return m_data + m_size; }

			const_iterator begin() const noexcept { return m_data; }
			const_iterator end() const noexcept { return m_data + m_size; }

			const_iterator cbegin() const noexcept { return m_data; }
			const_iterator cend() const noexcept { return m_data + m_size; }
		};




		 /** --- 便利エイリアス --- */
		template <typename T>
		using StackVector = std::vector<T, StackAdapter<T>>;

		template <typename T>
		using StackList = std::list<T, StackAdapter<T>>;
	}
}