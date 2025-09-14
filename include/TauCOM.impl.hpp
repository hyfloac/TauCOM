#pragma once

#include "TauCOM.hpp"
#include <atomic>

#ifdef TAU_COM_USE_TAU_UTILS
#include <allocator/TauAllocator.hpp>
#endif

#ifdef TAU_COM_USE_TAU_UTILS
    #define TAU_COM_DESTROY(PTR) BasicTauAllocator<AllocationTracking::None>::Instance().DeallocateT(PTR)
#else
    #define TAU_COM_DESTROY(PTR) delete (PTR)
#endif

#define TAU_COM_IMPL_REF_COUNT() \
    private: \
        ::std::atomic<::std::int32_t> m_AutoRefCount = 1; \
    public: \
        ::std::int32_t AddReference() noexcept override final { return ++m_AutoRefCount; } \
        ::std::int32_t ReleaseReference() noexcept override final { \
            const ::std::int32_t ret = m_AutoRefCount; \
            if((--m_AutoRefCount) <= 0) { \
                TAU_COM_DESTROY(this); \
            } \
            return ret; \
        }
