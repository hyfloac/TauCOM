// ReSharper disable CppInconsistentNaming
// ReSharper disable CppDFAUnreachableFunctionCall
#pragma once

#include <unordered_map>
#include <functional>
#include <cstdint>

#ifdef TAU_COM_USE_TAU_UTILS
#include <TauMacros.hpp>
#endif

#ifndef DYNAMIC_EXPORT
    #if defined(_WIN32)
      #define DYNAMIC_EXPORT __declspec(dllexport)
    #elif defined(__GNUC__) || defined(__clang__)
      #define DYNAMIC_EXPORT __attribute__((visibility("default")))
    #else
      #define DYNAMIC_EXPORT
    #endif
#endif

#ifndef DYNAMIC_IMPORT
    #if defined(_WIN32)
      #define DYNAMIC_IMPORT __declspec(dllimport)
    #elif defined(__GNUC__) || defined(__clang__)
      #define DYNAMIC_IMPORT
    #else
      #define DYNAMIC_IMPORT
    #endif
#endif

#ifdef TAU_COM_BUILD_SHARED
  #define TAU_COM_LIB DYNAMIC_EXPORT
#elif defined(TAU_COM_IMPORT_SHARED) || 1
  #define TAU_COM_LIB DYNAMIC_IMPORT
#elif defined(TAU_COM_BUILD_STATIC)
  #define TAU_COM_LIB
#endif

namespace tau::com {

struct UUID final
{
public:
    ::std::uint64_t Low;
    ::std::uint64_t High;
public:
    constexpr UUID() noexcept = default;

    constexpr UUID(const ::std::uint64_t low, const ::std::uint64_t high) noexcept
        : Low(low)
        , High(high)
    { }

    constexpr ~UUID() noexcept = default;

    constexpr UUID(const UUID& copy) noexcept = default;
    constexpr UUID(UUID&& move) noexcept = default;

    constexpr UUID& operator=(const UUID& copy) noexcept = default;
    constexpr UUID& operator=(UUID&& move) noexcept = default;

    [[nodiscard]] constexpr bool operator ==(const UUID& other) const noexcept { return Low == other.Low && High == other.High; }
    [[nodiscard]] constexpr bool operator !=(const UUID& other) const noexcept { return !((*this) == other); }
};

}

namespace std {

template<>
struct hash<::tau::com::UUID>
{
    [[nodiscard]] ::std::size_t operator()(const ::tau::com::UUID& uuid) const noexcept
    {
        if constexpr(sizeof(::std::size_t) == sizeof(::std::uint64_t))
        {
            return uuid.Low ^ uuid.High;
        }
        else if constexpr(sizeof(::std::size_t) == sizeof(::std::uint32_t))
        {
            const ::std::uint32_t lh = static_cast<::std::uint32_t>(uuid.Low >> 32);
            const ::std::uint32_t ll = static_cast<::std::uint32_t>(uuid.Low & 0xFFFFFFFF);
            const ::std::uint32_t hh = static_cast<::std::uint32_t>(uuid.High >> 32);
            const ::std::uint32_t hl = static_cast<::std::uint32_t>(uuid.High & 0xFFFFFFFF);
            return lh ^ ll ^ hh ^ hl;
        }
    }
};

}

#if __has_include(<EASTL/functional.h>)
#include <EASTL/functional.h>

namespace eastl {

template<>
struct hash<::tau::com::UUID>
{
    [[nodiscard]] ::std::size_t operator()(const ::tau::com::UUID& uuid) const noexcept
    {
        return ::std::hash<::tau::com::UUID>()(uuid);
    }
};

}
#endif

namespace tau::com {

// ReSharper disable once CppTemplateParameterNeverUsed
template<typename T>
struct ComUUID final
{
    // ReSharper disable once CppRedundantInlineSpecifier
    static inline constexpr UUID IID = UUID(0x0000000000000000ull, 0x0000000000000000ull);
};

#define TAU_DECL_UUID(T, LOW, HIGH) \
    namespace tau::com { \
    template<> \
    struct ComUUID<T> final { \
        static inline constexpr ::tau::com::UUID IID = ::tau::com::UUID((LOW), (HIGH)); \
    }; \
    }

template<typename T>
inline constexpr const UUID& uuid_of = ComUUID<T>::IID;

template<typename T>
inline constexpr const UUID& iid_of = ComUUID<T>::IID;

enum EResultCode : ::std::int32_t
{
    RC_Success = 0,
    RC_NullParam = -1,
    RC_InterfaceNotFound = -2,
    RC_Fail = -3,
    RC_InitializationError = -4,
    RC_InvalidParam = -5,
    RC_OutOfMemory = -6,
    RC_NotReady = -7,
    RC_FactoryAlreadyRegistered = 1,
    RC_Timeout = 2,
    RC_AsyncReturn = 3,
    RC_MoreItems = 4,
};

static bool IsSuccess(const EResultCode result) noexcept { return static_cast<::std::int32_t>(result) >= 0; }
static bool IsFailure(const EResultCode result) noexcept { return !IsSuccess(result); }

struct BaseConstructionInfo
{
public:
    UUID Iid;
    const BaseConstructionInfo* pNext;
public:
    BaseConstructionInfo() noexcept = default;
    virtual ~BaseConstructionInfo() noexcept = default;

    BaseConstructionInfo(const BaseConstructionInfo& copy) noexcept = default;
    BaseConstructionInfo(BaseConstructionInfo&& move) noexcept = default;

    BaseConstructionInfo& operator=(const BaseConstructionInfo& copy) noexcept = default;
    BaseConstructionInfo& operator=(BaseConstructionInfo&& move) noexcept = default;
};

class IUnknown
{
public:
    using ConstructionInfo = BaseConstructionInfo;
protected:
    IUnknown() noexcept = default;
public:
    virtual ~IUnknown() noexcept = default;
protected:
    IUnknown(const IUnknown& copy) noexcept = default;
    IUnknown(IUnknown&& move) noexcept = default;

    IUnknown& operator=(const IUnknown& copy) noexcept = default;
    IUnknown& operator=(IUnknown&& move) noexcept = default;
public:
    virtual ::std::int32_t AddReference() noexcept = 0;
    virtual ::std::int32_t ReleaseReference() noexcept = 0;

    virtual EResultCode QueryInterface(const UUID& iid, void** const pInterface) noexcept = 0;

    template<typename T>
    EResultCode QueryInterface(T** pInterface) noexcept
    {
        return QueryInterface(iid_of<T>, reinterpret_cast<void**>(pInterface));
    }
};


template<typename T>
class ComRef final
{
public:
    ComRef() noexcept
        : m_Ptr(nullptr)
    { }

    ComRef(T* const ptr) noexcept
        : m_Ptr(ptr)
    { }

    ComRef(::std::nullptr_t) noexcept
        : m_Ptr(nullptr)
    { }

    ~ComRef() noexcept
    {
        ReleaseReference();
    }

    ComRef(const ComRef<T>& copy) noexcept
        : m_Ptr(copy.m_Ptr)
    {
        AddReference();
    }

    ComRef(ComRef<T>&& move) noexcept
        : m_Ptr(move.m_Ptr)
    {
        move.m_Ptr = nullptr;
    }

    ComRef<T>& operator=(::std::nullptr_t) noexcept
    {
        ReleaseReference();

        m_Ptr = nullptr;

        return *this;
    }

    ComRef<T>& operator=(const ComRef<T>& copy) noexcept
    {
        if(this == &copy)
        {
            return *this;
        }

        ReleaseReference();

        m_Ptr = copy.m_Ptr;
        AddReference();

        return *this;
    }

    ComRef<T>& operator=(ComRef<T>&& move) noexcept
    {
        if(this == &move)
        {
            return *this;
        }

        ReleaseReference();

        m_Ptr = move.m_Ptr;
        move.m_Ptr = nullptr;

        return *this;
    }

    [[nodiscard]] operator T*() const noexcept { return m_Ptr; }
    [[nodiscard]] operator bool() const noexcept { return m_Ptr; }

    [[nodiscard]] T* operator->() const noexcept { return m_Ptr; }

    [[nodiscard]] T* Get() const noexcept { return m_Ptr; }
    [[nodiscard]] T** Load() noexcept { return &m_Ptr; }
    [[nodiscard]] void** LoadVoid() noexcept { return reinterpret_cast<void**>(&m_Ptr); }

    [[nodiscard]] bool operator==(const ComRef<T>& other) const noexcept { return m_Ptr == other.m_Ptr; }
    [[nodiscard]] bool operator!=(const ComRef<T>& other) const noexcept { return !(*this == other); }

    ::std::int32_t AddReference() noexcept
    {
        if(m_Ptr)
        {
            return m_Ptr->AddReference();
        }
        return 0;
    }

    ::std::int32_t ReleaseReference() noexcept
    {
        if(m_Ptr)
        {
            return m_Ptr->ReleaseReference();
        }
        return 0;
    }
private:
    T* m_Ptr;
};

class IComManager : public IUnknown
{
public:
    using ComFactoryFunc = EResultCode(*)(const UUID& iid, void** pInterface, const BaseConstructionInfo* const pConstructionInfo);
    using FactoryMap = ::std::unordered_map<UUID, ComFactoryFunc>;

    struct ConstructionInfo final : BaseConstructionInfo
    {
    public:
        FactoryMap Factories;
    public:
        ConstructionInfo() noexcept = default;
        ~ConstructionInfo() override = default;

        ConstructionInfo(const ConstructionInfo& copy) noexcept = default;
        ConstructionInfo(ConstructionInfo&& move) noexcept = default;

        ConstructionInfo& operator=(const ConstructionInfo& copy) noexcept = default;
        ConstructionInfo& operator=(ConstructionInfo&& move) noexcept = default;
    };
protected:
    IComManager() noexcept = default;
public:
    ~IComManager() noexcept override = default;
protected:
    IComManager(const IComManager& copy) noexcept = default;
    IComManager(IComManager&& move) noexcept = default;

    IComManager& operator=(const IComManager& copy) noexcept = default;
    IComManager& operator=(IComManager&& move) noexcept = default;
public:
    virtual EResultCode RegisterIidFactory(const UUID& iid, const ComFactoryFunc factory) noexcept = 0;
    virtual EResultCode CreateObject(const UUID& iid, void** const pInterface, const BaseConstructionInfo* const pConstructionInfo) noexcept = 0;

    template<typename T>
    // ReSharper disable once CppRedundantTypenameKeyword
    EResultCode CreateObject(T** const pInterface, const typename T::ConstructionInfo* const pConstructionInfo) noexcept
    {
        return CreateObject(iid_of<T>, reinterpret_cast<void**>(pInterface), static_cast<const BaseConstructionInfo*>(pConstructionInfo));
    }

    template<typename T>
    EResultCode CreateObject(T** const pInterface) noexcept
    {
        return CreateObject(iid_of<T>, reinterpret_cast<void**>(pInterface), nullptr);
    }
};

class IComManager1 : public IComManager
{
protected:
    IComManager1() noexcept = default;
public:
    ~IComManager1() noexcept override = default;
protected:
    IComManager1(const IComManager1& copy) noexcept = default;
    IComManager1(IComManager1&& move) noexcept = default;

    IComManager1& operator=(const IComManager1& copy) noexcept = default;
    IComManager1& operator=(IComManager1&& move) noexcept = default;
public:
    virtual EResultCode UnregisterIidFactory(const UUID& iid) noexcept = 0;
    virtual EResultCode GetIidFactory(const UUID& iid, ComFactoryFunc* const factory) noexcept = 0;
    virtual EResultCode Duplicate(IComManager1** const comManager) noexcept = 0;
};

}

TAU_DECL_UUID(::tau::com::IUnknown, 0x89D0171D1E547699ull, 0x3513C89A25664A40ull);
TAU_DECL_UUID(::tau::com::IComManager, 0xA84460A844FB841Cull, 0x8441F8C9B9F14C8Dull);
TAU_DECL_UUID(::tau::com::IComManager1, 0x2F6E3C1FFB854DD1ull, 0x8A17434B93524BB7ull);

extern "C" TAU_COM_LIB ::tau::com::EResultCode TauComGetComManager(::tau::com::IComManager** const pInterface) noexcept;
