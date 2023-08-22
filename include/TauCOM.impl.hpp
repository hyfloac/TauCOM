#pragma once

#include "TauCOM.hpp"
#include <atomic>
#include <allocator/TauAllocator.hpp>

#define TAU_COM_IMPL_REF_COUNT() \
    private: \
        ::std::atomic<::std::int32_t> m_AutoRefCount = 1; \
    public: \
        i32 AddReference() noexcept override final { return ++m_AutoRefCount; } \
        i32 ReleaseReference() noexcept override final { \
            const i32 ret = m_AutoRefCount; \
            if((--m_AutoRefCount) <= 0) { \
                BasicTauAllocator<AllocationTracking::None>::Instance().DeallocateT(this); \
            } \
            return ret; \
        }
