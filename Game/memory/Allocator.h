/**
 * Allocator.h
 * メモリ確保・解放の処理群
 */
#pragma once
#include <vector>
#include <list>
#include <mutex>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <typeinfo>
#include <memory>
#include <atomic>
#include <new>


#if defined(APP_DEBUG)
#define APP_ENABLE_DEBUG_ALLOCATOR
#endif

#if defined(APP_ENABLE_DEBUG_ALLOCATOR)
#define APP_USE_DUMP_ALLOCATOR
#endif // APP_USE_DEBUG_ALLOCATOR

namespace app
{
	namespace memory
	{
		/** 16byteアライメント */
		static constexpr size_t ALIGNMENT = 16;


		/**
		 * HeapAllocator
		 * アプリケーション全体で共有される親アロケーター
		 * - 起動時に一括確保
		 * - 解放時に空きブロックを結合(Coalesce)して断片化を防止
		 * - メモリリーク検知機能付き
		 * - スレッドセーフ
		 */
		class Allocator
		{
		private:
			struct alignas(ALIGNMENT) BlockHeader
			{
				size_t size = 0;					// データ部のサイズ
				bool isUsed = false;				// 使用中フラグ
				BlockHeader* prevPhys = nullptr;	// 物理的な前のブロック(結合用)
				BlockHeader* nextPhys = nullptr;	// 物理的な次のブロック(結合用)

#if defined(APP_ENABLE_DEBUG_ALLOCATOR)
				// デバッグ情報
				const char* fileName;
				int lineNum;
				const char* typeName;
				static constexpr uint32_t MAGIC = 0xDEADBEEF;
				uint32_t magic;
#endif 
			};
			static constexpr size_t HEADER_SIZE = (sizeof(BlockHeader) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);


		private:
			void* memoryStart_ = nullptr;
			size_t totalSize_ = 0;
			BlockHeader* freeListHead_ = nullptr;
			std::mutex mutex_;


		private:
			Allocator() {}


		public:
			/**
			 * 初期化 (アプリ起動時)
			 */
			void Initialize(size_t totalSize)
			{
				K2_ASSERT(totalSize_ == 0, "すでに初期化済みです\n");
				std::lock_guard<std::mutex> lock(mutex_);
				if (memoryStart_) return;

				memoryStart_ = std::malloc(totalSize);
				totalSize_ = totalSize;

				// 初期ブロック作成
				BlockHeader* first = static_cast<BlockHeader*>(memoryStart_);
				first->size = totalSize - HEADER_SIZE;
				first->isUsed = false;
				first->prevPhys = nullptr;
				first->nextPhys = nullptr;
#if defined(APP_ENABLE_DEBUG_ALLOCATOR)
				first->magic = BlockHeader::MAGIC;
				first->fileName = nullptr;
#endif // APP_ENABLE_DEBUG_ALLOCATOR

				freeListHead_ = first;

#if defined(APP_USE_DUMP_ALLOCATOR)
				K2_LOG("[Heap] Initialized %d KB.\n");
#endif // APP_USE_DUMP_ALLOCATOR
			}


			/**
			 * 終了処理 (アプリ終了時)
			 */
			void Shutdown()
			{
				std::lock_guard<std::mutex> lock(mutex_);
#if defined(APP_USE_DUMP_ALLOCATOR)
				DumpLeaks(); // リークチェック
#endif // APP_USE_DUMP_ALLOCATOR
				if (memoryStart_) {
					std::free(memoryStart_);
					memoryStart_ = nullptr;
				}
			}


			/**
			 * メモリ確保 (Alloc)
			 */
#if defined(APP_ENABLE_DEBUG_ALLOCATOR)
			void* Allocate(size_t size, const char* file = nullptr, int line = 0, const char* type = "Raw")
#else
			void* Allocate(size_t size)
#endif // APP_ENABLE_DEBUG_ALLOCATOR
			{
				std::lock_guard<std::mutex> lock(mutex_);

				size_t allocSize = (size + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
				BlockHeader* curr = freeListHead_;

				while (curr) {
					if (!curr->isUsed && curr->size >= allocSize) {
						// 必要であれば分割
						if (curr->size >= allocSize + HEADER_SIZE + ALIGNMENT) {
							SplitBlock(curr, allocSize);
						}
						curr->isUsed = true;
#if defined(APP_ENABLE_DEBUG_ALLOCATOR)
						curr->fileName = file;
						curr->lineNum = line;
						curr->typeName = type;
#endif // APP_ENABLE_DEBUG_ALLOCATOR

						// フリーリスト更新ロジック(簡易)
						if (curr == freeListHead_) freeListHead_ = curr->nextPhys;

						return reinterpret_cast<char*>(curr) + HEADER_SIZE;
					}
					curr = curr->nextPhys;
				}

#if defined(APP_USE_DUMP_ALLOCATOR)
				K2_LOG("[Heap] Out of Memory! Requested: %d \n", size);
#endif // APP_USE_DUMP_ALLOCATOR
				return nullptr;
			}


			/**
			 * メモリ解放 (Free)
			 */
			void Free(void* ptr)
			{
				if (!ptr) return;
				std::lock_guard<std::mutex> lock(mutex_);

				BlockHeader* header = reinterpret_cast<BlockHeader*>(static_cast<char*>(ptr) - HEADER_SIZE);
#if defined(APP_ENABLE_DEBUG_ALLOCATOR)
				K2_ASSERT(header->magic == BlockHeader::MAGIC, "メモリ破損検出\n");
				K2_ASSERT(header->isUsed == true, "二重解放の可能性があります\n");
#endif // APP_ENABLE_DEBUG_ALLOCATOR

				header->isUsed = false;

				// 結合。前後が空いていればくっつける
				if (header->nextPhys && !header->nextPhys->isUsed) Coalesce(header, header->nextPhys);
				if (header->prevPhys && !header->prevPhys->isUsed) Coalesce(header->prevPhys, header);

				// フリーリストの巻き戻し(簡易最適化)
				if (header < freeListHead_ || !freeListHead_) freeListHead_ = header;
				if (header->prevPhys && !header->prevPhys->isUsed && header->prevPhys < freeListHead_) {
					freeListHead_ = header->prevPhys;
				}
			}




		private:
			void SplitBlock(BlockHeader* block, size_t size)
			{
				size_t remaining = block->size - size - HEADER_SIZE;
				BlockHeader* newBlock = reinterpret_cast<BlockHeader*>(reinterpret_cast<char*>(block) + HEADER_SIZE + size);
				newBlock->size = remaining;
				newBlock->isUsed = false;
				newBlock->prevPhys = block;
				newBlock->nextPhys = block->nextPhys;
#if defined(APP_ENABLE_DEBUG_ALLOCATOR)
				newBlock->magic = BlockHeader::MAGIC;
				newBlock->fileName = nullptr;
#endif // APP_ENABLE_DEBUG_ALLOCATOR

				if (block->nextPhys) block->nextPhys->prevPhys = newBlock;
				block->nextPhys = newBlock;
				block->size = size;
			}


			void Coalesce(BlockHeader* first, BlockHeader* second)
			{
				first->size += second->size + HEADER_SIZE;
				first->nextPhys = second->nextPhys;
				if (second->nextPhys) second->nextPhys->prevPhys = first;
			}




#if defined(APP_USE_DUMP_ALLOCATOR)
		public:
			/**
			 * リーク表示
			 */
			void DumpLeaks()
			{
				if (!memoryStart_) return;
				BlockHeader* curr = static_cast<BlockHeader*>(memoryStart_);
				size_t count = 0;
				size_t bytes = 0;
				K2_LOG("\n=== MEMORY LEAK REPORT ===\n");
				while (curr) {
					if (curr->isUsed) {
						const char* f = curr->fileName ? curr->fileName : "Unknown";
						K2_LOG("Name:%s(Line:%d):[Type:%s] %d bytes.\n", f, curr->lineNum, curr->typeName, curr->size);
						count++;
						bytes += curr->size;
					}
					curr = curr->nextPhys;
				}
				if (count == 0) {
					K2_LOG("No Leaks Detected.\n");
				}
				else {
					K2_LOG("Total: %d leaks, %d bytes.\n", count, bytes);
				}
				K2_LOG("==========================\n");
			}
#endif // APP_USE_DUMP_ALLOCATOR




			/**
			 * シングルトン関連
			 */
		public:
			static Allocator& Get()
			{
				static Allocator instance;
				return instance;
			}
		};




		/**
		 * Thread Stack Context (内部クラス)
		 * スレッドごとに1つ存在するメモリの実体
		 */
		class ThreadStackContext
		{
		public:
			// スレッドごとのスタックサイズ（例: 2MB）
			// NOTE: 必要に応じて増やす
			static constexpr size_t THREAD_STACK_SIZE = 1024 * 1024 * 2;


		private:
			char* startPtr_ = nullptr;
			size_t totalSize_ = 0;
			size_t offset_ = 0; // スレッドローカルなのでatomic不要


		public:
			ThreadStackContext()
			{
				// ヒープから確保
				totalSize_ = THREAD_STACK_SIZE;
#if defined(APP_ENABLE_DEBUG_ALLOCATOR)
				startPtr_ = static_cast<char*>(Allocator::Get().Allocate(totalSize_, "ThreadStack", 0, "ThreadStackContext"));
#else
				startPtr_ = static_cast<char*>(Allocator::Get().Allocate(totalSize_));
#endif
			}

			~ThreadStackContext()
			{
				// スレッド終了時に返却
				if (startPtr_) Allocator::Get().Free(startPtr_);
			}
			/** コピー禁止 */
			ThreadStackContext(const ThreadStackContext&) = delete;
			ThreadStackContext& operator=(const ThreadStackContext&) = delete;


			void* Alloc(size_t size, size_t alignment)
			{
				size_t currentAddr = reinterpret_cast<size_t>(startPtr_) + offset_;
				size_t padding = (alignment - (currentAddr % alignment)) % alignment;

				if (offset_ + padding + size > totalSize_) {
					// スタックオーバーフロー
					K2_ASSERT(false, "[ThreadStack] Out of Memory! Requested: %d \n", size);
					return nullptr;
				}

				void* ptr = startPtr_ + offset_ + padding;
				offset_ += padding + size;
				return ptr;
			}

			size_t GetMarker() const { return offset_; }
			void FreeToMarker(size_t marker) { offset_ = marker; }
			size_t GetUsed() const { return offset_; }




		public:
			/** シングルトン取得（Thread Local） */
			static ThreadStackContext& Get()
			{
				// URL:https://cpprefjp.github.io/lang/cpp11/thread_local_storage.html
				static thread_local ThreadStackContext instance;
				return instance;
			}
		};




		/**
		 * StackAllocatorMarker
		 * NOTE: RAII（Resource Acquisition Is Initialization）
		 * 実体は持たず、スコープ内でのメモリ管理を行う。
		 */
		class StackAllocatorMarker
		{
		private:
			/** コンストラクタ呼び出し時のオフセット位置 */
			size_t m_marker;


		public:
			StackAllocatorMarker()
			{
				//現在のスタック位置を保存する
				m_marker = ThreadStackContext::Get().GetMarker();
			}
			~StackAllocatorMarker()
			{
				// ここで保存しておいた位置まで巻き戻す（スコープ内の確保分を解放）
				ThreadStackContext::Get().FreeToMarker(m_marker);
			}
			/** コピー禁止 */
			StackAllocatorMarker(const StackAllocatorMarker&) = delete;
			StackAllocatorMarker& operator=(const StackAllocatorMarker&) = delete;


			/** メモリ確保 */
			void* Alloc(size_t size, size_t alignment = ALIGNMENT)
			{
				return ThreadStackContext::Get().Alloc(size, alignment);
			}
		};




		/**
		 * StackAllocatorをstd::vectorなどで使うためのアダプター
		 */
		template <typename T>
		class StackAdapter
		{
		public:
			using value_type = T;


		public:
			StackAllocatorMarker* stack_;


		public:
			StackAdapter(StackAllocatorMarker& stack) : stack_(&stack) {}
			template <typename U> StackAdapter(const StackAdapter<U>& other) : stack_(other.stack_) {}

			T* allocate(size_t n)
			{
				T* p = static_cast<T*>(stack_->Alloc(n * sizeof(T), alignof(T)));
				if (!p) {
					// STLコンテナ用にbad_allocを投げる
					// NOTE: bad_alloc=new演算子がメモリ確保に失敗したときに投げられる例外クラス
					throw std::bad_alloc();
				}
				return p;
			}
			void deallocate(T*, size_t) {} // Stackなので解放しない
			bool operator==(const StackAdapter& other) const { return stack_ == other.stack_; }
			bool operator!=(const StackAdapter& other) const { return !(*this == other); }
		};
	}
}